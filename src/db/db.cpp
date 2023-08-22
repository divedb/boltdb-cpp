#include "boltdb/db/db.hpp"

#include <vector>

#include "boltdb/fs/file_system.hpp"
#include "boltdb/os/darwin.hpp"
#include "boltdb/util/binary.hpp"
#include "boltdb/util/crc64.hpp"

namespace boltdb {

DB::DB(DB&& rhs) noexcept { move_aux(other); }

void DB::init() {
  std::vector<Page> pages;
  std::ostringstream oss;

  for (int i = 0; i < 2; i++) {
    Page page(i, PageFlag::kMeta, page_size_);
    Meta* meta = page.meta();
    meta->magic = kMagic;
    meta->version = kVersion;
    meta->page_size = page_size_;
    meta->freelist = 2;
    meta->flags = 0;
    meta->root = {.root = 3, .sequence = 0};
    meta->pgid = i;
    meta->txid = i;

    pages.push_back(page);
  }
}

DB& DB::operator=(DB&& other) noexcept {
  move_aux(other);

  return *this;
}

void DB::move_aux(DB&& other) noexcept {
  if (this == &other) {
    return;
  }

  // TODO(gc): fix move
  options_ = other.options_;
  file_handle_ = std::move(other.file_handle_);
  lock_file_ = other.lock_file_;
  dataref_ = other.dataref_;
  data_size_ = other.data_size_;
  file_size_ = other.file_size_;
  meta0_ = other.meta0_;
  meta1_ = other.meta1_;
  page_size_ = other.page_size_;
  opened_ = other.opened_;
  rwtx_ = other.rwtx_;
  txns_ = other.txns_;
}

[[nodiscard]] Status open(std::string path, int permission, Options options,
                          DB** out_db) {
  auto handle = FileSystem::open(path.c_str(), options.open_flag(), permission);

  if (handle == nullptr) {
    return Status(StatusType::kStatusErr, "Fail to open: " + path);
  }

  // Lock file so that other processes using Bolt in read-write mode cannot use
  // the database at the same time. This would cause corruption since the two
  // processes would write meta pages and free pages separately.
  // The database file is locked exclusively (only one process can grab the
  // lock) if !options.ReadOnly.
  // The database file is locked using the shared lock (more than one process
  // may hold a lock at the same time) otherwise (options.ReadOnly is set).
  int operation = LOCK_EX;

  if (options.is_read_only()) {
    operation = LOCK_SH;
  }

  Status status = handle->flock(operation, options.timeout());

  if (!bool(status)) {
    return status;
  }

  if (FileSystem::file_size(*handle) == 0) {
  }
}

// Serialize the given meta object.
ByteSlice Meta::serialize(const Meta& meta) {
  ByteSlice slice;

  meta.write_with_checksum(slice);
  
  return slice;
}

template <typename T>
std::span<Byte> deserialize_aux(std::span<Byte> data, T& out_data) {
  std::size_t offset = sizeof(T);
  out_data = binary::BigEndian::uint<T>(data);

  return data.subspan(offset);
}

// Deserialize meta from the given slice.
Meta Meta::deserialize(ByteSlice slice) {
  Meta meta;
  std::span<Byte> sp = slice.span();

  sp = deserialize_aux(sp, meta.magic);
  sp = deserialize_aux(sp, meta.version);
  sp = deserialize_aux(sp, meta.page_size);
  sp = deserialize_aux(sp, meta.flags);
  sp = deserialize_aux(sp, meta.root.root);
  sp = deserialize_aux(sp, meta.root.sequence);
  sp = deserialize_aux(sp, meta.freelist);
  sp = deserialize_aux(sp, meta.pgid);
  sp = deserialize_aux(sp, meta.txid);
  sp = deserialize_aux(sp, meta.checksum);

  return meta;
}

void Meta::compute_checksum() const {
  ByteSlice slice;

  write_without_checksum(slice);
  checksum = crc64(0, slice.data(), slice.size());
}

void Meta::write_with_checksum(ByteSlice slice) {
  write_without_checksum(slice);
  binary::BigEndian::append_uint(slice, checksum);
}

void Meta::write_without_checksum(ByteSlice slice) const {
  binary::BigEndian::append_uint(slice, magic);
  binary::BigEndian::append_uint(slice, version);
  binary::BigEndian::append_uint(slice, page_size);
  binary::BigEndian::append_uint(slice, flags);
  binary::BigEndian::append_uint(slice, root.root);
  binary::BigEndian::append_uint(slice, root.sequence);
  binary::BigEndian::append_uint(slice, freelist);
  binary::BigEndian::append_uint(slice, pgid);
  binary::BigEndian::append_uint(slice, txid);
}


}  // namespace boltdb