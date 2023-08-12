#ifndef BOLTDB_CPP_FS_FILE_SYSTEM_HPP_
#define BOLTDB_CPP_FS_FILE_SYSTEM_HPP_

#include <fcntl.h>
#include <stdint.h>

#include <memory>
#include <string_view>

#include "boltdb/util/types.hpp"

namespace boltdb {

class FileHandle;

// Flags to open wrapping those of the underlying system.
// Not all flags may be implemented on a given system.
enum Flag : int {
  O_RDONLY = 0x00000000,  // open for reading only
  O_WRONLY = 0x00000001,  // open for writing only
  O_RDWR = 0x00000002,    // open for reading and writing
  // The remaining values may be or'ed in to control behavior.
  O_APPEND = 0x00000008,  // append date to the file when writing
  O_CREATE = 0x00000200,  // create a new file if none exists
  O_TRUNC = 0x00000400,   // truncate regular writable file when opened
  O_EXCL = 0x00000800,    // used wit O_CREATE, file must not exist
  O_SYNC = 0x00000080,    // open for synchronous I/O
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
  static std::unique_ptr<FileHandle> open(const char* path, Flag mode,
                                          int perm) noexcept;

  // Check if the given file corresponds to an existing file or directory.
  // Return true if the given path or file status corresponds to an existing
  // file or directory, false otherwise.
  static bool exists(FileHandle& handle) noexcept;
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_FS_FILE_SYSTEM_HPP_
