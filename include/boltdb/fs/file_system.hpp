#ifndef BOLTDB_CPP_FS_FILE_SYSTEM_HPP_
#define BOLTDB_CPP_FS_FILE_SYSTEM_HPP_

#include <fcntl.h>
#include <unistd.h>

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>

#include "boltdb/util/status.hpp"
#include "boltdb/util/types.hpp"

namespace boltdb {

// This is a basic abstraction of file handle.
// Since we need flock on the specified file based on the file descriptor
// it seems no portable way to get file descriptor from `ifstream`.
class FileHandle {
 public:
  explicit FileHandle(int fd, const char* path) : fd_(fd), path_(path) {}
  ~FileHandle() noexcept;

  [[nodiscard]] int fd() const { return fd_; }
  [[nodiscard]] std::string path() const { return path_; }

  // Applies or removes an advisory lock on the file associated with the file
  // descriptor fd.
  // A lock is applied by specifying an operation parameter that is one of
  // LOCK_SH or LOCK_EX with the optional addition of LOCK_NB.
  // To unlock an existing lock operation should be LOCK_UN.
  // man 2 flock:
  //    Requesting a lock on an object that is already locked normally causes
  //    the caller to be blocked until the lock may be acquired. If LOCK_NB is
  //    included in operation, then this will not happen; instead the call will
  //    fail and the error EWOULDBLOCK will be returned.
  // NOTES
  //  Locks are on files, not file descriptors.  That is, file descriptors
  //  duplicated through dup(2) or fork(2) do not result in multiple instances
  //  of a lock, but rather multiple references to a single lock.  If a process
  //  holding a lock on a file forks and the child explicitly unlocks the file,
  //  the parent will lose its lock.
  //
  //  Processes blocked awaiting a lock may be awakened by signals.
  [[nodiscard]] Status flock(int operation, double timeout_s);

 private:
  bool flocked_;
  int fd_;
  std::string path_;
};

// The `FileSystem` class provides facilities for performing operations on file
// systems and their components, such as paths, regular files, and directories.
class FileSystem {
 public:
  // Creates or truncates the named file. If the file already exists, it is
  // truncated. If the file does not exist, it is created with mode 0666 (before
  // unmask). If successful, methods on the returned FileHandle can be used for
  // I/O; the associated file descriptor has mode O_RDWR. If there is an error,
  // nullptr will be returned.
  // TODO(gc): change to gsl::zstring later
  static std::unique_ptr<FileHandle> create(const char* path) noexcept;
  static std::unique_ptr<FileHandle> open(const char* path, int oflag,
                                          int permission) noexcept;

  // Check if the given file corresponds to an existing file or directory.
  // Return true if the given path or file status corresponds to an existing
  // file or directory, false otherwise.
  static bool exists(FileHandle& handle);

  // Remove the file or directory specified by path.
  // If path specifies a directory, remove(path) is equivalent of rmdir(path).
  // Otherwise, it is the equivalent of unlink(path).
  // Return `StatusOK` if successfull; otherwise `StatusErr` with error message.
  static Status remove(FileHandle& handle);
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_FS_FILE_SYSTEM_HPP_
