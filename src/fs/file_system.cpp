#include "boltdb/fs/file_system.hpp"

#include <sys/file.h>

#include <cerrno>
#include <chrono>
#include <cstdio>
#include <string>
#include <thread>

namespace boltdb {

Status FileHandle::flock(int operation, double timeout) const {
  auto start = std::chrono::high_resolution_clock::now();

  while (true) {
    int res = ::flock(fd_, operation);

    if (res == 0) {
      break;
    }

    // Check error first.
    if (res != EWOULDBLOCK) {
      return Status(StatusType::kStatusErr, strerror(errno));
    }

    // Check timeout.
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed =
        std::chrono::duration_cast<std::chrono::duration<double>>(now - start)
            .count();

    if (timeout > 0 && elapsed > timeout) {
      return Status(StatusType::kStatusErr, "Flock timeout");
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  return Status{};
}

std::unique_ptr<FileHandle> FileSystem::create(const char* path) noexcept {
  return open(path, O_RDWR | O_CREAT | O_TRUNC, 0666);
}

std::unique_ptr<FileHandle> FileSystem::open(const char* path, int oflag,
                                             int permission) noexcept {
  int fd = ::open(path, oflag, permission);

  if (fd == -1) {
    return nullptr;
  }

  return std::make_unique<FileHandle>(fd, path);
}

bool FileSystem::exists(FileHandle& handle) {
  std::string path = handle.path();

  return access(path.c_str(), F_OK) == 0;
}

Status FileSystem::remove(FileHandle& handle) {
  std::string path = handle.path();

  int res = std::remove(path.c_str());

  if (res == -1) {
    return Status(StatusType::kStatusErr, strerror(res));
  }

  return Status{};
}

}  // namespace boltdb