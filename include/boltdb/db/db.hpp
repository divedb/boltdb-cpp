#ifndef BOLTDB_CPP_DB_HPP_
#define BOLTDB_CPP_DB_HPP_

#include <fstream>
#include <memory>
#include <vector>

#include "boltdb/fs/file_system.hpp"
#include "boltdb/page/page.hpp"
#include "boltdb/storage/bucket.hpp"
#include "boltdb/transaction/txn.hpp"
#include "boltdb/util/common.hpp"
#include "boltdb/util/options.hpp"
#include "boltdb/util/status.hpp"
#include "boltdb/util/types.hpp"

namespace boltdb {

class Txn;

class Meta {
 public:
  // Serialize the given meta object.
  static ByteSlice serialize(const Meta& meta);

  // Deserialize meta from the given slice.
  static Meta deserialize(ByteSlice slice);

  // Write meta information to the specified slice.
  // This does *include* the checksum field.
  void write(ByteSlice& slice) const;

  // Compute checksum.
  u64 sum64() const;

  // Checks the marker bytes and version of the meta page to ensure it matches
  // this binary.
  Status validate() const;

  bool equals(const Meta& other) const;

  u32 magic;
  u32 version;
  u32 page_size;
  u32 flags;
  BucketMeta root;
  PageID freelist;  // Freelist page id
  PageID pgid;      // Meta page id
  TxnID txid;       // Transaction id
  u64 checksum;

 private:
  void write_aux(ByteSlice& slice) const;
};

// DB represents a collection of buckets persisted to a file on disk.
// All data access is performed through transactions which can be obtained
// through the DB. All the functions on DB will return a ErrDatabaseNotOpen if
// accessed before Open() is called.
class DB {
 public:
  // The largest step that can be taken when remapping the mmap.
  constexpr static const int kMaxMmapStep = 1 << 30;  // 1GB

  // The data file format version.
  constexpr static const u32 kVersion = 1;

  // Represents a marker value to indicate that a file is a Bolt DB.
  constexpr static const u32 kMagic = 0xED0CDAED;

  // TODO(gc): fix this and free memory
  ~DB() {}

  // Disallow copy and assignment constructor.
  DB(const DB&) = delete;
  DB& operator=(const DB&) = delete;

  // Move only.
  DB(DB&& other) noexcept;
  DB& operator=(DB&& other) noexcept;

  // Get the path to currently open database file.
  std::string path() const { return file_handle_->path; }

  // Get a page reference from the mmap based on the current page size.
  Page* page(PageID pgid) const;

  friend Status open_db(std::string path, Options options, DB** out_db);

 private:
  DB(std::unique_ptr<FileHandle> file_handle, Options options)
      : file_handle_(std::move(file_handle)), options_(options) {}

  void move_aux(DB&& other) noexcept;

  // Initialize the meta, freelist and root pages.
  Status init() const;

  std::unique_ptr<FileHandle> file_handle_;
  Options options_;

  FileHandle* lock_file_;  // windows only
  Byte* dataref_;          // mmap'ed readonly, write throws SEGV
  Byte* data_;
  int data_size_;
  int file_size_;  // current on disk file size
  Meta meta0_;
  Meta meta1_;
  int page_size_;
  bool opened_;
  Txn* rwtx_;
  std::vector<Txn*> txns_;
};

// Open a database at the specified path.
// If the file does not exist then it will be created automatically.
Status open_db(std::string path, Options options, DB** out_db);

}  // namespace boltdb

#endif  // BOLTDB_CPP_DB_HPP_