#include "boltdb/db/db.hpp"

#include <vector>

#include "boltdb/fs/file_system.hpp"

namespace boltdb {

namespace {

// Initialize the meta, freelist and root pages.
bool init(FileHandle& handle, u32 page_size) {
  std::vector<Byte> buf(page_size * 4);

  for (int i = 0; i < 2; i++) {
    Page* page = reinterpret_cast<Page*>(&buf[i * page_size]);
  }
}

}  // namespace

DB::DB(DB&& rhs) noexcept { move_aux(other); }

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

}  // namespace boltdb