#ifndef BOLTDB_CPP_PAGE_FREE_LIST_HPP_
#define BOLTDB_CPP_PAGE_FREE_LIST_HPP_

#include <functional>
#include <map>
#include <numeric>
#include <vector>

#include "boltdb/transaction/txn.hpp"
#include "boltdb/util/common.hpp"
#include "boltdb/util/status.hpp"

namespace boltdb {

// FreeList represents a list of all pages that are available for allocation.
// It also tracks pages that have been freed but are still in use by open
// transactions.
class FreeList {
 public:
  FreeList() = default;

  FreeList(std::vector<PageID> ids) : ids_(std::move(ids)) {}

  // Get the size of the page after serialization in bytes.
  int size() const;

  // Get count of pages on the freelist.
  int count() const { return free_count() + pending_count(); }

  // Get count of free pages.
  int free_count() const { return ids_.size(); }

  // Get count of pending pages.
  int pending_count() const {
    return std::accumulate(
        pending_.begin(), pending_.end(), 0,
        [](int init, const auto& entry) { return init + entry.second.size(); });
  }

  // Get the starting page id of a contiguous list of pages of a given size.
  // If a contiguous block cannot be found then 0 is returned.
  PageID allocate_contiguous(int n);

  // Release a page and its overflow for a given transaction id.
  // If the page is already free then a panic will occur.
  void free(TxnID txn_id, Page* p);

  // Moves all page ids for a transaction id (or older) to the freelist.
  void release(TxnID txn_id);

  // Removes the pages from the given pending tx.
  void rollback(TxnID txn_id);

  // Return true if the specified page id is in the free list.
  // Otherwise return false.
  bool is_freed(PageID pgid) const;

  // Return true if the specified page id is in the pending list of the given
  // transaction id.
  // Otherwise return false.
  bool is_pending(TxnID txn_id, PageID pgid) const;

  // Initializes the freelist from a freelist page.
  void read_from(Page* page);

  // Writes the page ids onto a freelist page. All free and pending ids are
  // saved to disk since in the event of a program crash, all pending ids will
  // become free.
  Status write_to(Page* page);

  // Reads the freelist from a page and filters out pending items.
  void reload(Page* page);

 private:
  static constexpr const u16 kSpecialCount = 0xFFFF;

  // Rebuild the free cache based on available and pending free lists.
  void reindex();

  // Get sorted pending page ids held by all the transactions.
  std::vector<PageID> sorted_pending_pgids() const;

  // Get sorted pending page ids held by the specified transaction id
  // and also older transaction (with smaller transaction ids).
  std::vector<PageID> sorted_pending_pgids(TxnID txn_id) const;

  std::vector<PageID> sorted_pending_pgids_impl(
      const std::function<bool(TxnID)>& pred) const;

  // Merge the specified sorted free page ids into `ids_`.
  void merge(const std::vector<PageID>& sorted_free_pgids);

  // All free and available free page ids.
  std::vector<PageID> ids_;
  // Mapping of soon-to-be free page ids by transaction.
  std::map<TxnID, std::vector<PageID>> pending_;
  // Fast lookup of all free and pending page ids.
  std::map<PageID, bool> cache_;
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_PAGE_FREE_LIST_HPP_
