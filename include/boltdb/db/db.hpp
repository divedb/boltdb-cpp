#ifndef BOLTDB_CPP_DB_HPP_
#define BOLTDB_CPP_DB_HPP_

#include <fstream>
#include <string_view>
#include <vector>

#include "boltdb/db/file_handle.hpp"
#include "boltdb/storage/page.hpp"
#include "boltdb/transaction/transaction.hpp"
#include "boltdb/util/common.hpp"
#include "boltdb/util/options.hpp"
#include "boltdb/util/status.hpp"
#include "boltdb/util/types.hpp"

namespace boltdb {

class Meta;
class Tx;

// DB represents a collection of buckets persisted to a file on disk.
// All data access is performed through transactions which can be obtained
// through the DB. All the functions on DB will return a ErrDatabaseNotOpen if
// accessed before Open() is called.
class DB {
 public:
  // Get the path to currently open database file.
  std::string path() const;

 private:
  Options options_;
  std::string path_;
  FileHandle* file_;
  FileHandle* lock_file_;  // windows only
  Byte* dataref_;          // mmap'ed readonly, write throws SEGV
  int data_size_;
  int file_size_;  // current on disk file size
  Meta* meta0_;
  Meta* meta1_;
  int page_size_;
  bool opened_;
  Tx* rwtx_;
  std::vector<Tx*> txs_;
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

// Open a database at the specified path.
// If the file does not exist then it will be created automatically.
[[nodiscard]] Status open(std::string path, int permission, Options options,
                          DB** out_db);

}  // namespace boltdb

#endif  // BOLTDB_CPP_DB_HPP_