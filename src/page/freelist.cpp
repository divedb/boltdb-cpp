#include "boltdb/page/freelist.hpp"

#include "boltdb/page/page.hpp"
#include "boltdb/util/exception.hpp"

namespace boltdb {

// TODO(gc): makes this template function more general, not limited to vector.
template <typename T>
std::vector<T> merge_two(const std::vector<T>& v1, const std::vector<T>& v2) {
  std::vector<T> result(v1.size() + v2.size());

  std::merge(v1.begin(), v1.end(), v2.begin(), v2.end(), result.begin());

  return result;
}

int FreeList::size() const {
  auto n = count();

  if (n >= kSpecialCount) {
    // The first element will be used to store the count.
    // See freelist.write.
    n++;
  }

  return kPageHeaderSize + sizeof(PageID) * n;
}

// TODO(gc): make the code more readable.
PageID FreeList::allocate_contiguous(int n) {
  if (ids_.empty()) {
    return 0;
  }

  PageID prev_id = 0;
  auto first = ids_.begin();

  for (auto iter = ids_.begin(); iter != ids_.end(); iter++) {
    PageID id = *iter;

    if (id <= 1) {
      std::string error = format("invalid page allocation: %d", id);
      throw DBException(error);
    }

    // Reset initial page if this is not contiguous.
    // Don't need to check `prev_id == 0` condition since id must >= 2.
    if (id - prev_id != 1) {
      first = iter;
    }

    // If we found a contiguous block then remove it and return it.
    if (std::distance(first, iter) + 1 == n) {
      auto first_id = *first;

      ids_.erase(first, iter);

      // Remove from the free cache.
      for (int j = 0; j < n; j++) {
        cache_.erase(j + first_id);
      }

      return first_id;
    }

    prev_id = id;
  }

  return 0;
}

void FreeList::free(TxnID txn_id, Page* p) {
  // TODO(gc): why this is 1, not 2 since we have 2 meta pages and 1 freelist
  // page.
  auto id = p->id();

  if (id <= 1) {
    std::string error = format("cannot free page 0 or 1: %d", id);
    throw DBException(error);
  }

  // Free page and all its overflow pages.
  for (auto i = id; i <= id + p->overflow(); i++) {
    if (cache_.find(i) != cache_.end()) {
      std::string error = format("page %d already freed", i);
      throw DBException(error);
    }

    pending_[txn_id].push_back(i);
    cache_[i] = true;
  }
}

void FreeList::release(TxnID txn_id) {
  std::vector<TxnID> txn_ids;
  txn_ids.reserve(pending_.size());

  // Sort the pending page ids and collect the transaction ids that needs to be
  // removed.
  auto pgids = sorted_pending_pgids_impl([&txn_ids, txn_id](TxnID id) {
    if (id <= txn_id) {
      txn_ids.push_back(id);

      return true;
    }

    return false;
  });

  for (auto id : txn_ids) {
    pending_.erase(id);
  }

  ids_ = merge_two(pgids, ids_);
}

void FreeList::rollback(TxnID txn_id) {
  // Remove page ids from cache.
  for (auto id : pending_[txn_id]) {
    cache_.erase(id);
  }

  // Remove pages from pending list.
  // TODO(gc): how about the modification on the pages.
  pending_.erase(txn_id);
}

bool FreeList::is_freed(PageID pgid) const {
  return cache_.find(pgid) != cache_.end();
}

// TODO(gc): do we need to update `pending_`.
void FreeList::read_from(Page* page) {
  int count = page->count();
  Byte* base = page->skip_page_header();

  if (count == kSpecialCount) {
    count = *reinterpret_cast<PageID*>(base);
    base = std::next(base, sizeof(PageID));
  }

  // Copy the list of page ids from the freelist.
  if (count == 0) {
    ids_.clear();
  } else {
    auto first = reinterpret_cast<PageID*>(base);
    auto last = std::next(first, count);
    ids_.resize(count);

    std::copy(first, last, ids_.begin());
    std::sort(ids_.begin(), ids_.end());
  }

  reindex();
}

Status FreeList::write_to(Page* page) {
  // Update the header flag.
  page->set_flag(kFreeList);

  // The page.count can only hold up to 64K elements so if we overflow that
  // number then we handle it by putting the size in the first element.

  // Suppose the page size is 4K and size of PageID is 8,
  // the maximum number of ids is 512 (ignore the page header).
  int n = count();
  Byte* base = page->skip_page_header();
  auto first = reinterpret_cast<PageID*>(base);

  if (n < kSpecialCount) {
    page->set_count(n);
  } else {
    page->set_count(kSpecialCount);
    *first = n;
    first = std::next(first, sizeof(PageID));
  }

  std::vector<PageID> pgids = merge_two(ids_, sorted_pending_pgids());
  std::copy(pgids.begin(), pgids.end(), first);

  return {};
}

void FreeList::reload(Page* page) {
  read_from(page);

  // Build a cache of only pending pages.
  std::map<PageID, bool> pcache;

  for (auto&& [_, pgids] : pending_) {
    for (auto pgid : pgids) {
      pcache[pgid] = true;
    }
  }

  // Check each page in the freelist and build a new available freelist with any
  // pages not int the pending lists.
  // Note that, we could do this in place.
  auto last = std::copy_if(
      ids_.begin(), ids_.end(), ids_.begin(),
      [&pcache](PageID id) { return pcache.find(id) == pcache.end(); });

  ids_.resize(std::distance(ids_.begin(), last));

  // Once the available list is rebuilt then rebuild the free cache so that it
  // includes the available and pending free pages.
  reindex();
}

void FreeList::reindex() {
  for (auto id : ids_) {
    cache_[id] = true;
  }

  for (auto&& [_, pgids] : pending_) {
    for (PageID pgid : pgids) {
      cache_[pgid] = true;
    }
  }
}

std::vector<PageID> FreeList::sorted_pending_pgids() const {
  return sorted_pending_pgids_impl([](TxnID) { return true; });
}

std::vector<PageID> FreeList::sorted_pending_pgids(TxnID txn_id) const {
  return sorted_pending_pgids_impl(
      [txn_id](TxnID tid) { return tid <= txn_id; });
}

std::vector<PageID> FreeList::sorted_pending_pgids_impl(
    const std::function<bool(TxnID)>& pred) const {
  int n = pending_count();
  std::vector<PageID> result(n);
  auto d_first = std::back_inserter(result);

  for (auto&& [_, pgid_vec] : pending_) {
    d_first = std::copy_if(pgid_vec.begin(), pgid_vec.end(), d_first,
                           [pred](TxnID tid) { return pred(tid); });
  }

  std::sort(result.begin(), result.end());

  return result;
}

}  // namespace boltdb