#ifndef BOLTDB_CPP_DB_OPTIONS_HPP_
#define BOLTDB_CPP_DB_OPTIONS_HPP_

#include <fcntl.h>

#include "boltdb/util/types.hpp"

namespace boltdb {

// Options represents the options that can be set when opening a database.
class Options {
 public:
  constexpr static const int kDefaultMaxBatchSize = 1000;
  constexpr static const int kDefaultMaxBatchDelay = 10;
  constexpr static const int kDefaultAllocSize = 16 * 1024 * 1024;

  // Accessor
  bool is_strict_mode() const { return strict_mode_; }
  bool is_no_sync() const { return no_sync_; }
  bool is_no_grow_sync() const { return no_grow_sync_; }
  bool is_read_only() const { return read_only_; }

  int permission() const { return permission_; }
  int open_flag() const { return open_flag_; }
  double timeout() const { return timeout_; }
  int mmap_flags() const { return mmapflags_; }
  int initial_mmap_size() const { return initial_mmap_size_; }
  int max_batch_size() const { return max_batch_size_; }
  int max_batch_delay() const { return max_batch_delay_; }
  int alloc_size() const { return alloc_size_; }

  // Modifier
  Options& set_permission(int permission) {
    permission_ = permission;
    return *this;
  }

  Options& set_open_flag(int open_flag) {
    open_flag_ = open_flag;
    return *this;
  }

  Options& set_strict_mode(bool strict_mode) {
    strict_mode_ = strict_mode;
    return *this;
  }

  Options& set_no_sync(bool no_sync) {
    no_sync_ = no_sync;
    return *this;
  }

  Options& set_no_grow_sync(bool no_grow_sync) {
    no_grow_sync_ = no_grow_sync;
    return *this;
  }

  Options& set_read_only(bool read_only) {
    read_only_ = read_only;
    return *this;
  }

  Options& set_timeout(double timeout) {
    timeout_ = timeout;
    return *this;
  }

  Options& set_mmap_flags(int mmapflags) {
    mmapflags_ = mmapflags;
    return *this;
  }

  Options& set_initial_mmap_size(int initial_mmap_size) {
    initial_mmap_size_ = initial_mmap_size;
    return *this;
  }

  Options& set_max_batch_size(int max_batch_size) {
    max_batch_size_ = max_batch_size;
    return *this;
  }

  // Set max batch delay in milli seconds.
  Options& set_max_batch_delay(int max_batch_delay_ms) {
    max_batch_delay_ = max_batch_delay_ms;
    return *this;
  }

  Options& set_alloc_size(int alloc_size) {
    alloc_size_ = alloc_size;
    return *this;
  }

 private:
  // The flags specified for this argument must include exactly one of the
  // following file access modes:
  // O_RDONLY: open for reading only
  // O_WRONLY: open for writing only
  // O_RDWR: open for reading and writingc
  // O_SEARCH: open directory for searching (this may depend on the OS)
  // O_EXEC: open for execute only
  // In addition any combination of the following values can be or'ed in
  // `open_flag`:
  // O_NONBLOCK: do not block on open or for data to become available
  // O_APPEND: append on each write
  // O_CREAT: create file if it does not exist
  // O_TRUNC: truncate size to 0
  // O_EXCL: error if O_CREAT and the file exists
  // O_SHLOCK: atomically obtain a shared lock
  // O_EXLOCK: atomically obtain an exclusive lock
  // O_DIRECTORY: restrict open to a directory
  // O_NOFOLLOW: do not follow symlinks
  // O_SYMLINK: allow open of symlinks
  // O_EVTONLY: descriptor requested for event notifications only
  // O_CLOEXEC: mark as close-on-exec
  // O_NOFOLLOW_ANY: do not follow symlinks in the entire path
  int open_flag_{O_RDWR};

  int permission_{0666};

  // When enabled, the database will perform a Check() after every commit.
  // A panic is issued if the database is in an inconsistent state. This
  // flag has a large performance impact so it should only be used for
  // debugging purposes.
  bool strict_mode_{};

  // Setting the NoSync flag will cause the database to skip fsync()
  // calls after each commit. This can be useful when bulk loading data
  // into a database and you can restart the bulk load in the event of
  // a system failure or database corruption. Do not set this flag for
  // normal use.
  //
  // If the package global IgnoreNoSync constant is true, this value is
  // ignored.  See the comment on that constant for more details.
  //
  // THIS IS UNSAFE. PLEASE USE WITH CAUTION.
  bool no_sync_{true};

  // When true, skips the truncate call when growing the database.
  // Setting this to true is only safe on non-ext3/ext4 systems.
  // Skipping truncation avoids preallocation of hard drive space and
  // bypasses a truncate() and fsync() syscall on remapping.
  //
  // https://github.com/boltdb/bolt/issues/284
  bool no_grow_sync_{};

  // Open database in read-only mode. Use flock(..., LOCK_SH | LOCK_NB) to
  // grab a shared lock (UNIX).
  bool read_only_{};

  // Timeout is the amount of time to wait to obtain a file lock.
  // When set to zero it will wait indefinitely. This option is only
  // available on Darwin and Linux.
  double timeout_{};

  // If you want to read the entire database fast, you can set MmapFlag to
  // syscall.MAP_POPULATE on Linux 2.6.23+ for sequential read-ahead.
  int mmapflags_{};

  // InitialMmapSize is the initial mmap size of the database
  // in bytes. Read transactions won't block write transaction
  // if the InitialMmapSize is large enough to hold database mmap
  // size. (See DB.Begin for more information)
  //
  // If <= 0, the initial map size is 0.
  // If initialMmapSize is smaller then the previous database size.
  // it takes no effect.
  int initial_mmap_size_{};

  // MaxBatchSize is the maximum size of a batch. Default value is
  // copied from DefaultMaxBatchSize in Open.
  //
  // If <=0, disables batching.
  //
  // Do not change concurrently with calls to Batch.
  int max_batch_size_{kDefaultMaxBatchSize};

  // MaxBatchDelay is the maximum delay before a batch starts.
  // Default value is copied from DefaultMaxBatchDelay in Open.
  //
  // If <=0, effectively disables batching.
  //
  // Do not change concurrently with calls to Batch.
  int max_batch_delay_{kDefaultMaxBatchDelay};

  // AllocSize is the amount of space allocated when the database
  // needs to create new pages. This is done to amortize the cost
  // of truncate() and fsync() when growing the data file.
  int alloc_size_{kDefaultAllocSize};
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_DB_OPTIONS_HPP_
