#include "boltdb/page/freelist.hpp"

#include "boltdb/page/page.hpp"
#include "boltdb/util/exception.hpp"

namespace boltdb {

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
    // Don't need to check `prev_id == 0` condition since id must >= 2
    if (id - prev_id != 1) {
      first = iter;
    }

    // If we found a contiguous block then remove it and return it.
    if (std::distance(first, iter) + 1 == n) {
      ids_.erase(first, iter);

      // Remove from the free cache.
      for (int j = 0; j < n; j++) {
        cache_.erase(j + initial);
      }

      return *first;
    }

    prev_id = id;
  }

  return 0;
}

void FreeList::free(TxnID txid, Page* p) {
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

    pending_[txid].push_back(i);
    cache_[i] = true;
  }
}

void FreeList::release(TxnID txid) {
  std::vector<PageID> sorted_pgids;
  std::vector<TxnID> erase_txids;

  std::for_each(pending_.begin(), pending_.end(), [&](auto&& iter) {
    if (iter.first <= txid) {
      erase_txids.push_back(iter.first);
      sorted_pgids.insert(sorted_pgids.end(), iter.second.begin(),
                          iter.second.end());
    }
  });

  std::sort(sorted_pgids.begin(), sorted_pgids.end());

  for (auto&& id : erase_txids) {
    pending_.erase(id);
  }

  std::vector<PageID> tmp(ids_);
  ids_.resize(tmp.size() + sorted_pgids.size());
  std::merge(tmp.begin(), tmp.end(), sorted_pgids.begin(), sorted_pgids.end(),
             ids_.begin());
}

void FreeList::rollback(TxnID txid) {
  // Remove page ids from cache.
  for (auto id : pending_[txid]) {
    cache_.erase(id);
  }

  // Remove pages from pending list.
  // TODO(gc): how about the modification on the pages.
  pending_.erase(txid);
}

bool FreeList::is_freed(PageID pgid) const {
  return cache_.find(pgid) != cache_.end();
}

void FreeList::read_from(Page* page) {
  int index = 0;
  int count = page->count();
  Byte* base = page->skip_page_header();

  if (count == kSpecialCount) {
    index = 1;
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
    // Make sure they're sorted.
    std::sort(ids_.begin(), ids_.end());
  }

  // Rebuild the page cache.
  reindex();
}

Status FreeList::write_to(Page* page) {
  // Update the header flag.
  page->set_flag(kFreeList);

  // The page.count can only hold up to 64K elements so if we overflow that
  // number then we handle it by putting the size in the first element.

  // Suppose the page size is 4K and size of PageID is 8,
  // the maximum number of ids is 512 (ignore the page header).
  int cnt = count();
  Byte* base = page->skip_page_header();
  PageID* first = reinterpret_cast<PageID*>(base);

  if (cnt < kSpecialCount) {
    page->set_count(cnt);
  } else {
    page->set_count(kSpecialCount);
    *first = cnt;
    first = std::next(first, sizeof(PageID));
  }

  std::vector<PageID> pgids = sorted_free_pages();
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
  std::vector<PageID> tmp;
  tmp.reserve(ids_.size());

  std::copy_if(
      ids_.begin(), ids_.end(), std::back_insert_iterator(tmp),
      [&pcache](PageID id) { return pcache.find(id) == pcache.end(); });

  ids_ = tmp;

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