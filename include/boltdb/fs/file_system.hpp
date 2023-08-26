#ifndef BOLTDB_CPP_FS_FILE_SYSTEM_HPP_
#define BOLTDB_CPP_FS_FILE_SYSTEM_HPP_

#include <fcntl.h>
#include <unistd.h>

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>

#include "boltdb/util/slice.hpp"
#include "boltdb/util/status.hpp"
#include "boltdb/util/types.hpp"

namespace boltdb {

class FileSystem;

// This is a basic abstraction of file handle.
// Since we need flock on the specified file based on the file descriptor
// it seems no portable way to get file descriptor from `ifstream`.
class FileHandle {
 public:
  explicit FileHandle(std::string path) : path(std::move(path)) {}

  virtual ~FileHandle() {}

  // Return the number of bytes have been read.
  virtual ssize_t read(void* out_buffer, std::size_t nbytes,
                       std::size_t offset) = 0;
  virtual void write(void* in_buffer, std::size_t nbytes,
                     std::size_t offset) = 0;
  virtual void close() = 0;

  // fdatasync() is similar to fsync(), but does not flush modified metadata
  // unless that metadata is needed order to allow a subsequent data retrieval
  // to be correctly handled. For example, changes to `st_atime`or `st_mtime`
  // (respectively, time of last access and time of last modification; see
  // inode(7)) do not require flushing because not necessary for a subsequent
  // data read to be handled correctly. On the other hand, a change to the file
  // size (`st_size`, as made by say `ftruncate(2)`), would require a metadata
  // flush.
  virtual Status fdatasync() = 0;

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
  virtual Status flock(int operation, double timeout_s) = 0;

  std::string path;
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

  // Get size of the file and return -1 if failed.
  // TODO(gc): get size of directory
  static std::uintmax_t file_size(FileHandle& handle);

 private:
  // Set the file pointer of a file handle to a specified location.
  // Reads and writes will happen from this location.
  void set_file_pointer(FileHandle& handle, std::size_t offset);
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_FS_FILE_SYSTEM_HPP_
