#include "boltdb/fs/file_system.hpp"

#include <string>

namespace boltdb {

std::unique_ptr<FileHandle> FileSystem::create(const char* path) noexcept {
  return open(path, O_RDWR | O_CREAT | O_TRUNC, 0666);
}

std::unique_ptr<FileHandle> FileSystem::open(const char* path, int mode,
                                             int perm) noexcept {
  int fd = ::open(path, mode, perm);

  if (fd == -1) {
    return nullptr;
  }

  return std::make_unique<FileHandle>(fd, path);
}

bool FileSystem::exists(FileHandle& handle) {
  std::string path = handle.path();

  return access(path.c_str(), F_OK) == 0;
}

}  // namespace boltdb