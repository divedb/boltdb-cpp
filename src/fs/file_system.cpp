#include "boltdb/fs/file_system.hpp"

#include <sys/file.h>
#include <sys/stat.h>

#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <string>
#include <thread>

#include "boltdb/util/timer.hpp"
#include "util/exception.hpp"
#include "util/util.hpp"

namespace boltdb {

class UnixFileHandle : public FileHandle {
 public:
  UnixFileHandle(std::string path, int fd) : FileHandle(path), fd_(fd) {}

  virtual ~UnixFileHandle() { close(); }

  Status flock(int operation, double timeout_s) override {
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
        return {StatusType::kStatusErr, "flock timeout"};
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    flocked_ = true;

    return {};
  }

  void close() override {
    if (flocked_) {
      flock(fd_, LOCK_UN);
    }

    if (fd_ != -1) {
      close(fd_);
    }
  }

  int fd() const { return fd_; }

 private:
  int fd_;
  bool flocked_;
};

// TODO(gc): do we need to cache the `offset`. And if the offset is same with
// current position, there is no need to fseek.
Status FileHandle::write_at(ByteSlice slice, std::size_t offset) {
  int res = std::fseek(fp_, offset, SEEK_SET);

  if (res != 0) {
    return {StatusType::kStatusErr, format("fseek: %s", strerror(errno))};
  }

  std::size_t size = slice.size();
  std::size_t nbytes = fwrite(slice.data(), sizeof(Byte), size, fp_);

  if (nbytes != size) {
    return {StatusType::kStatusErr, format("fwrite: %ld/%ld", nbytes, size};
  }

  return {};
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

  return std::make_unique<UnixFileHandle>(fd, path);
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

void FileSystem::set_file_pointer(FileHandle& handle, std::size_t offset) {
  int fd = static_cast<UnixFileHandle&>(handle).fd();
  off_t offset = lseek(fd, offset, SEEK_SET);

  if (offset == static_cast<off_t>(-1)) {
    std::message error = format("Could not seek to location %lld for file \"%s\": %s", offset, handle.path.c_str(),
      strerror(errno)));
  }
}

}  // namespace boltdb