#include "boltdb/db/db.hpp"

#include <vector>

#include "boltdb/fs/file_system.hpp"
#include "boltdb/os/darwin.hpp"
#include "boltdb/util/binary.hpp"
#include "boltdb/util/crc64.hpp"
#include "boltdb/util/exception.hpp"
#include "boltdb/util/util.hpp"

namespace boltdb {

Status DB::init() const {
  std::vector<Page> pages;

  // Create two meta pages.
  for (int i = 0; i < 2; i++) {
    Page page(i, PageFlag::kMeta, page_size_);
    Meta* meta = page.meta();

    // Initialize the meta page.
    meta->magic = kMagic;
    meta->version = kVersion;
    meta->page_size = page_size_;
    meta->freelist = 2;
    meta->flags = 0;
    meta->root = {.root = 3, .sequence = 0};
    meta->pgid = i;
    meta->txid = i;
    meta->compute_checksum();

    pages.push_back(page);
  }

  // Write an empty freelist at page 3.
  pages.emplace_back(2, PageFlag::kFreeList, page_size_);

  // Write an empty leaf page at page 4.
  pages.emplace_back(3, PageFlag::kLeaf, page_size_);

  // Write the first 4 pages to the data file.
  ssize_t offset = 0;

  try {
    for (Page& page : pages) {
      ssize_t bytes_written =
          file_handle_->write(page.data(), page_size_, offset);

      if (bytes_written != page_size_) {
        std::string error =
            format("write: expect written %d bytes, got %d bytes",
                   bytes_written, page_size_);
        return {kStatusCorrupt, error};
      }

      offset += page_size_;
    }

    file_handle_->fdatasync();
  } catch (const IOException& e) {
    return {kStatusErr, e.what()};
  }

  return {};
}

Status open_db(std::string path, Options options, DB** out_db) {
  auto handle = FileSystem::open(path.c_str(), options.open_flag() | O_CREAT,
                                 options.permission());

  if (handle == nullptr) {
    return {StatusType::kStatusErr, "error: fail to open " + path};
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

  std::unique_ptr<DB> db(new DB(std::move(handle), options));

  if (FileSystem::file_size(*db->file_handle_) == 0) {
    Status status = db->init();

    if (!status.ok()) {
      return status;
    }
  }

  *out_db = db.release();

  return {};
}

// Serialize the given meta object.
ByteSlice Meta::serialize(const Meta& meta) {
  ByteSlice slice;

  meta.write(slice);

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

void Meta::compute_checksum() {
  ByteSlice slice;

  write_without_checksum(slice);
  checksum = crc64_be(0, slice.data(), slice.size());
}

void Meta::write(ByteSlice& slice) const {
  write_without_checksum(slice);
  slice = binary::BigEndian::append_uint(slice, checksum);
}

void Meta::write_without_checksum(ByteSlice& slice) const {
  slice = binary::BigEndian::append_uint(slice, magic);
  slice = binary::BigEndian::append_uint(slice, version);
  slice = binary::BigEndian::append_uint(slice, page_size);
  slice = binary::BigEndian::append_uint(slice, flags);
  slice = binary::BigEndian::append_uint(slice, root.root);
  slice = binary::BigEndian::append_uint(slice, root.sequence);
  slice = binary::BigEndian::append_uint(slice, freelist);
  slice = binary::BigEndian::append_uint(slice, pgid);
  slice = binary::BigEndian::append_uint(slice, txid);
}

}  // namespace boltdb