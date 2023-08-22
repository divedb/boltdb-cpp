#include "boltdb/fs/file_system.hpp"

#include <sys/file.h>
#include <sys/stat.h>

#include <cerrno>
#include <chrono>
#include <cstdio>
#include <string>
#include <thread>
#include <cstring>

#include "boltdb/util/timer.hpp"

namespace boltdb {

FileHandle::~FileHandle() noexcept {
  if (flocked_) {
    // TODO(gc): fix this.
    try {
      (void)flock(LOCK_UN, 0);
    } catch (...) {
    }
  }

  close(fd_);
}

Status FileHandle::flock(int operation, double timeout) {
  Timer timer(timeout);

  while (true) {
    int res = ::flock(fd_, operation);

    if (res == 0) {
      break;
    }

    // Check error first.
    if (res != EWOULDBLOCK) {
      return {StatusType::kStatusErr, strerror(errno)};
    }

    // Check timeout.
    if (timeout > 0 && timer.is_timeout()) {
      return {StatusType::kStatusErr, "Flock timeout"};
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  flocked_ = true;

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
    return {StatusType::kStatusErr, strerror(res)};
  }

  return Status{};
}

std::uintmax_t FileSystem::file_size(FileHandle& handle) {
  struct stat st;
  int fd = handle.fd();
  int res = fstat(fd, &st);

  if (res == -1) {
    return static_cast<std::uintmax_t>(-1);
  }

  return st.st_size;
}

}  // namespace boltdb