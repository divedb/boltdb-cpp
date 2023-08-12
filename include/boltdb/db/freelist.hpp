#ifndef BOLTDB_CPP_DB_FREE_LIST_HPP_
#define BOLTDB_CPP_DB_FREE_LIST_HPP_

#include <map>
#include <vector>

#include "boltdb/transaction/transaction.hpp"
#include "boltdb/util/common.hpp"

namespace boltdb {

// FreeList represents a list of all pages that are available for allocation.
// It also tracks pages that have been freed but are still in use by open transactions.
class FreeList {
 public:
  // Get the size of the page after serialization.
  int size() const;

  // Get count of pages on the freelist.
  int count() const;

  // Get count of free pages.
  int free_count() const;

  // Get count of pending pages.
  int pending_count() const;

  std::vector<PageID> sorted_pending_pages() const;

  // Copies into dst of all free ids and all pending ids in one sorted list.
  std::vector<PageID> sorted_free_pages() const;

  // Get the starting page id of a contiguous list of pages of a given size.
  // If a contiguous block cannot be found then 0 is returned.
  PageID allocate_contiguous(int n);

  // Release  a page and its overflow for a given transaction id.
  // If the page is already free then a panic will occur.
  void free(TxID txid, Page* p);

  // Moves all page ids for a transaction id (or older) to the freelist.
  // TODO(gc): 为什么需要移除旧的
  void release(TxID txid);

  // Removes the pages from the given pending tx.
  void rollback(TxID txid);

  // Check whether a given page is in the free list.
  bool is_freed(PageID pid) const;

  // Initializes the freelist from a freelist page.
  void read(Page* page);

  std::vector<PageID> _ids;                      // all free and available free page ids
  std::map<TxID, std::vector<PageID>> _pending;  // mapping of soon-to-be free page ids by transaction
  std::map<PageID, bool> _cache;                 // fast lookup of all free and pending page ids.
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_DB_FREE_LIST_HPP_
