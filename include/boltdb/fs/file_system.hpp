#ifndef BOLTDB_CPP_FS_FILE_SYSTEM_HPP_
#define BOLTDB_CPP_FS_FILE_SYSTEM_HPP_

#include <fcntl.h>
#include <unistd.h>

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include "boltdb/util/types.hpp"

namespace boltdb {

// This is a basic abstraction of file handle.
// Since we need flock on the specified file based on the file descriptor
// it seems no portable way to get file descriptor from `ifstream`.
class FileHandle {
 public:
  explicit FileHandle(int fd, const char* path) : fd_(fd), path_(path) {}
  ~FileHandle() { close(fd_); }

  [[nodiscard]] int fd() const { return fd_; }
  [[nodiscard]] std::string path() const { return path_; }

 private:
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
  static std::unique_ptr<FileHandle> open(const char* path, int mode,
                                          int perm) noexcept;

  // Check if the given file corresponds to an existing file or directory.
  // Return true if the given path or file status corresponds to an existing
  // file or directory, false otherwise.
  static bool exists(FileHandle& handle);
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_FS_FILE_SYSTEM_HPP_
