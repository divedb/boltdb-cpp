#ifndef BOLTDB_CPP_DB_HPP_
#define BOLTDB_CPP_DB_HPP_

#include <fstream>
#include <memory>
#include <vector>

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
  // The largest step that can be taken when remapping the mmap.
  constexpr static const int kMaxMmapStep = 1 << 30;  // 1GB

  // The data file format version.
  constexpr static const int kVersion = 1;

  // Represents a marker value to indicate that a file is a Bolt DB.
  constexpr static const int kMagic = 0xED0CDAED;

  DB(std::unique_ptr<FileHandle> file_handle, Options options)
      : file_handle_(std::move(file_handle)), options_(options) {}

  // Move only.
  DB(DB&& other) noexcept;
  DB& operator=(DB&& other) noexcept;

  DISALLOW_COPY(DB);

  // Get the path to currently open database file.
  [[nodiscard]] std::string path() const { return file_handle_->path(); }

 private:
  void move_aux(DB&& other) noexcept;

  // Initialize the meta, freelist and root pages.
  void init();

  Options options_;
  std::unique_ptr<FileHandle> file_handle_;
  FileHandle* lock_file_;  // windows only
  Byte* dataref_;          // mmap'ed readonly, write throws SEGV
  int data_size_;
  int file_size_;  // current on disk file size
  Meta meta0_;
  Meta meta1_;
  int page_size_;
  bool opened_;
  Tx* rwtx_;
  std::vector<Tx*> txns_;
};

class Meta {
 public:
  // Serialize the given meta object.
  static ByteSlice serialize(const Meta& meta);

  // Deserialize meta from the given slice.
  static Meta deserialize(ByteSlice slice);

  void compute_checksum() const;

  // Write meta information to the specified slice.
  // This does *include* the checksum field.
  void write_with_checksum(ByteSlice slice);

  // Write meta information to the specified slice.
  // This does *exclude* the checksum field.
  void write_without_checksum(ByteSlice slice) const;

  bool equals(const Meta& rhs) const {
    return magic == rhs.meta && version == rhs.version && page_size == rhs.page_size && flags == rhs.flags &&
      root.root == rhs.root.root && root.sequence == rhs.root.sequence && freelist == rhs.freelist && pgid == rhs.pgid &&
      txid == rhs.txid && checksum == rhs.checksum;
  } 

  u32 magic;
  u32 version;
  u32 page_size;
  u32 flags;
  BucketMeta root;
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