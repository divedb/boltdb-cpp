#ifndef BOLTDB_CPP_DB_OPTIONS_HPP_
#define BOLTDB_CPP_DB_OPTIONS_HPP_

#include "boltdb/util/common.hpp"

namespace boltdb {

// Options represents the options that can be set when opening a database.
class Options {
 public:
  // Accessor
  bool is_strict_mode() const { return _strict_mode; }
  bool is_no_sync() const { return _no_sync; }
  bool is_no_grow_sync() const { return _no_grow_sync; }
  bool is_read_only() const { return _read_only; }

  Duration timeout() const { return _timeout; }
  int mmap_flags() const { return _mmapflags; }
  int initial_mmap_size() const { return _initial_mmap_size; }

  // Modifier
  Options& set_strict_mode(bool strict_mode) {
    _strict_mode = strict_mode;
    return *this;
  }

  Options& set_no_sync(bool no_sync) {
    _no_sync = no_sync;
    return *this;
  }

  Options& set_no_grow_sync(bool no_grow_sync) {
    _no_grow_sync = no_grow_sync;
    return *this;
  }

  Options& set_read_only(bool read_only) {
    _read_only = read_only;
    return *this;
  }

  Options& set_timeout(Duration timeout) {
    _timeout = timeout;
    return *this;
  }

  Options& set_mmap_flags(int mmapflags) {
    _mmapflags = mmapflags;
    return *this;
  }

  Options& set_initial_mmap_size(int initial_mmap_size) {
    _initial_mmap_size = initial_mmap_size;
    return *this;
  }

 private:
  // When enabled, the database will perform a Check() after every commit.
  // A panic is issued if the database is in an inconsistent state. This
  // flag has a large performance impact so it should only be used for
  // debugging purposes.
  bool _strict_mode{};

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
  bool _no_sync{true};

  // When true, skips the truncate call when growing the database.
  // Setting this to true is only safe on non-ext3/ext4 systems.
  // Skipping truncation avoids preallocation of hard drive space and
  // bypasses a truncate() and fsync() syscall on remapping.
  //
  // https://github.com/boltdb/bolt/issues/284
  bool _no_grow_sync{};

  // Open database in read-only mode. Use flock(..., LOCK_SH | LOCK_NB) to
  // grab a shared lock (UNIX).
  bool _read_only;

  // Timeout is the amount of time to wait to obtain a file lock.
  // When set to zero it will wait indefinitely. This option is only
  // available on Darwin and Linux.
  Duration _timeout{};

  // If you want to read the entire database fast, you can set MmapFlag to
  // syscall.MAP_POPULATE on Linux 2.6.23+ for sequential read-ahead.
  int _mmapflags;

  // InitialMmapSize is the initial mmap size of the database
  // in bytes. Read transactions won't block write transaction
  // if the InitialMmapSize is large enough to hold database mmap
  // size. (See DB.Begin for more information)
  //
  // If <= 0, the initial map size is 0.
  // If initialMmapSize is smaller then the previous database size.
  // it takes no effect.
  int _initial_mmap_size;

  // MaxBatchSize is the maximum size of a batch. Default value is
  // copied from DefaultMaxBatchSize in Open.
  //
  // If <=0, disables batching.
  //
  // Do not change concurrently with calls to Batch.
  int _max_batch_size;

  // MaxBatchDelay is the maximum delay before a batch starts.
  // Default value is copied from DefaultMaxBatchDelay in Open.
  //
  // If <=0, effectively disables batching.
  //
  // Do not change concurrently with calls to Batch.
  Duration _max_batch_delay;

  // AllocSize is the amount of space allocated when the database
  // needs to create new pages. This is done to amortize the cost
  // of truncate() and fsync() when growing the data file.
  int _alloc_size;
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_DB_OPTIONS_HPP_
