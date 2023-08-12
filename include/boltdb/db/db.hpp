#ifndef BOLTDB_CPP_DB_HPP_
#define BOLTDB_CPP_DB_HPP_

#include <fstream>
#include <string_view>
#include <vector>

#include "boltdb/db/file_handle.hpp"
#include "boltdb/db/options.hpp"
#include "boltdb/storage/page.hpp"
#include "boltdb/transaction/transaction.hpp"
#include "boltdb/util/common.hpp"

namespace boltdb {

class Meta;
class Tx;

// DB represents a collection of buckets persisted to a file on disk.
// All data access is performed through transactions which can be obtained
// through the DB. All the functions on DB will return a ErrDatabaseNotOpen if
// accessed before Open() is called.
class DB {
 public:
  DB(std::string_view path, )

  // Get the path to currently open database file.
  std::string path() const;

 private:
  Options _options;
  std::string _path;
  FileHandle* _file;
  FileHandle* _lock_file;  // windows only
  Byte* _dataref;          // mmap'ed readonly, write throws SEGV
  int _data_size;
  int _file_size;  // current on disk file size
  Meta* _meta0;
  Meta* _meta1;
  int _page_size;
  bool _opened;
  Tx* _rwtx;
  std::vector<Tx*> _txs;
};

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