#include "boltdb/db/freelist.hpp"

#include <algorithm>
#include <numeric>

#include "boltdb/storage/page.hpp"
#include "boltdb/util/const.hpp"
#include "boltdb/util/exception.hpp"
#include "boltdb/util/util.hpp"

using namespace boltdb;

int FreeList::size() const {
  int n = count();

  // En: the first element will be used to store the count. See freelist.write.
  // Ch: 如果第一个值大于等于0xFFFF 那么这个值就是freelist的个数
  if (n >= 0xFFFF) {
    n++;
  }

  return kPageHeaderSize + n * sizeof(PageID);
}

int FreeList::count() const { return free_count() + pending_count(); }

int FreeList::free_count() const { return _ids.size(); }

int FreeList::pending_count() const {
  return std::accumulate(_pending.begin(), _pending.end(), 0,
                         [](int init, const auto& entry) { return init + entry.second.size(); });
}

std::vector<PageID> FreeList::sorted_pending_pages() const {
  int n = pending_count();
  std::vector<PageID> sorted(n);
  auto output = sorted.begin();

  for (auto&& [_, pid_vec] : _pending) {
    output = std::copy(pid_vec.begin(), pid_vec.end(), output);
  }

  return sorted;
}

std::vector<PageID> FreeList::sorted_free_pages() const {
  auto sorted = sorted_pending_pages();

  return std::merge(_ids.begin(), _ids.end(), sorted.begin(), sorted.end());
}

PageID FreeList::allocate_contiguous(int n) {
  decltype(_ids.begin()) initial;
  int previd = kInvalidPageID;

  for (auto i = _ids.begin(); i != _ids.end(); i++) {
    int id = *i;

    // TODO(gc): does this because the first 2 pages are meta pages
    if (id <= 1) {
      throw DBException(format("invalid page allocation: %d", id));
    }

    // Reset initial page if this is not contiguous.
    if (previd == kInvalidPageID || id - previd != 1) {
      initial = i;
    }

    // If we found a contiguous block then remove it and return it.
    if (std::distance(initial, i) + 1 == n) {
      // Works for Go.
      // If we're allocating off the beginning then take the fast path
      // and just adjust the existing slice. This will be use extra memory
      // temporarily but the append() in free() will realloc the slice
      // as is necessary.
      _ids.erase(initial, ++i);

      // Remove from the free cache.
      for (auto j = initial; j != i; j++) {
        _cache.erase(*j);
      }

      return *initial;
    }
  }

  return kInvalidPageID;
}

void FreeList::free(TxID txid, Page* p) {
  int pid = p->pid();

  if (pid < kAvailPageID) {
    std::string err = format("cannot free page 0 or 1: %d", pid);
    throw DBException(err);
  }

  // Free page and all its overfloa pages.
  // TODO(gc): overflow表示的是个数还是page id以及为什么小于等于
  auto& pending_ids = _pending[txid];

  for (auto i = pid; i <= pid + p->overflow(); i++) {
    // Verify that page is not already free.
    if (_cache.find(i) != _cache.end()) {
      std::string err = format("page %d already freed", i);
      throw DBException(err);
    }

    // Add to the freelist and cache.
    pending_ids.push_back(i);
    _cache[i] = true;
  }
}

void FreeList::release(TxID txid) {
  std::vector<PageID> buf;

  for (auto&& [tid, ids] : _pending) {
    if (tid <= txid) {
      // Move transaction's pending pages to the available freelist.
      // Don't remove from the cache since the page is still free.
      buf.insert(buf.end(), ids.begin(), ids.end());
    }
  }

  std::remove_if(_pending.begin(), _pending.end(), [txid](const auto&& entry) { return entry.first <= txid; });
  std::sort(buf.begin(), buf.end());
  _ids = std::merge(_ids, buf);
}

void FreeList::rollback(TxID txid) {
  // Remove page ids from cache.
  for (auto&& id : _pending[txid]) {
    _cache.erase(id);
  }

  // Remove pages from pending list.
  _pending.erase(txid);
}

inline bool FreeList::is_freed(PageID pid) const { return _cache.find(pid) != _cache.end(); }

void FreeList::read(Page* page) {}