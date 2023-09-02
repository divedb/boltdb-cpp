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
    meta->checksum = meta->sum64();

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
    compute_checksum return {kStatusErr, e.what()};
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
    db->page_size_ = OS::getpagesize();
    Status status = db->init();

    if (!status.ok()) {
      return status;
    }
  } else {
    Status status;
    // Read the first meta page to determine the page size.
    ByteSlice buffer(0x1000);

    // TODO(gc): what happens if the page size is 512, 1024, 2048, 4096, 8192.
    ssize_t bytes_read = db->file_handle_->read(
        const_cast<Byte*>(buffer.data()), buffer.size(), 0);
    Meta meta = Meta::deserialize(buffer);

    if (status = meta.validate(); !status.ok()) {
      // If we can't read the page size, we can assume it's the same
      // as the OS -- since that's how the page size was chosen in the
      // first place.
      //
      // If the first page is invalid and this OS uses a different
      // page size than what the database was created with then we
      // are out of luck and cannot access the database.
      db->page_size_ = OS::getpagesize();
    } else {
      db->page_size_ = meta.page_size;
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
void deserialize_aux(ByteSlice& slice, T& out_data) {
  out_data = binary::BigEndian::uint<T>(slice);
  slice.remove_prefix(sizeof(T));
}

// Deserialize meta from the given slice.
Meta Meta::deserialize(ByteSlice slice) {
  Meta meta;

  deserialize_aux(slice, meta.magic);
  deserialize_aux(slice, meta.version);
  deserialize_aux(slice, meta.page_size);
  deserialize_aux(slice, meta.flags);
  deserialize_aux(slice, meta.root.root);
  deserialize_aux(slice, meta.root.sequence);
  deserialize_aux(slice, meta.freelist);
  deserialize_aux(slice, meta.pgid);
  deserialize_aux(slice, meta.txid);
  deserialize_aux(slice, meta.checksum);

  return meta;
}

u64 Meta::sum64() const {
  ByteSlice slice;
  write_aux(slice);

  return crc64_be(0, slice.data(), slice.size());
}

void Meta::write(ByteSlice& slice) const {
  write_aux(slice);
  slice = binary::BigEndian::append_uint(slice, checksum);
}

void Meta::write_aux(ByteSlice& slice) const {
  slice = binary::BigEndian::append_variadic_uint(
      slice, magic, version, page_size, flags, root.root, root.sequence,
      freelist, pgid, txid);
}

}  // namespace boltdb