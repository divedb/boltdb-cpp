#include "boltdb/transaction/txn.hpp"

namespace boltdb {

Page* Txn::page(PageID pgid) {
  if (pages_.find(pgid) != pages_.end()) {
    return pages_[pgid];
  }

  // Otherwise return directly from the mmap.
  return db_->page(pgid);
}

Status Txn::allocate(int count, Page*& out_page) {}

}  // namespace boltdb