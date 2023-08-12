#include "boltdb/fs/file_system.hpp"

#include <unistd.h>

#include <string>

using namespace boltdb;

namespace boltdb {

// A simple abstraction of FILE.
// The reason to do this it seems no portable way to get file descriptor
// from `ifstream`. And we need flock on the file based on the file descriptor.
class FileHandle {
 public:
  explicit FileHandle(int fd, const char* path) : _fd(fd), _path(path) {}
  ~FileHandle() { close(_fd); }

  int fd() const { return _fd; }
  std::string path() const { return _path; }

 private:
  int _fd;
  std::string _path;
};

}  // namespace boltdb

std::unique_ptr<FileHandle> FileSystem::create(const char* path) noexcept {
  return open(path, Flag::O_RDWR | Flag::O_CREATE | Flag::O_TRUNC, 0666);
}

std::unique_ptr<FileHandle> FileSystem::open(const char* path, Flag mode,
                                             int perm) noexcept {
  int fd = ::open(path, int(mode), perm);

  if (fd == -1) {
    return nullptr;
  }

  return std::make_unique<FileHandle>(fd);
}

bool FileSystem::exists(FileHandle& handle) noexcept {
  std::string path = handle.path();

  return access(path.c_str(), F_OK) == 0;
}
