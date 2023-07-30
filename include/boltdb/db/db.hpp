#ifndef BOLTDB_CPP_DB_HPP_
#define BOLTDB_CPP_DB_HPP_

#include "boltdb/storage/page.hpp"
#include "boltdb/transaction/transaction.hpp"
#include "boltdb/util/common.hpp"

namespace boltdb {

class bucket;

struct Meta {
  u32 magic;
  u32 version;
  u32 page_size;
  u32 flags;
  bucket root;
  PageID freelist;  // Freelist page id
  PageID pgid;      // Meta page id
  TxnID txid;       // Transaction id
  u64 checksum;
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_DB_HPP_