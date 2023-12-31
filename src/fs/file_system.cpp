#include "boltdb/fs/file_system.hpp"

#include <sys/file.h>
#include <sys/stat.h>

#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <string>
#include <thread>

#include "boltdb/util/exception.hpp"
#include "boltdb/util/timer.hpp"
#include "boltdb/util/util.hpp"

namespace boltdb {

class UnixFileHandle final : public FileHandle {
 public:
  UnixFileHandle(std::string path, int fd)
      : FileHandle(std::move(path)), fd_(fd) {}

  ~UnixFileHandle() override { close(); }

  ssize_t read(void* out_buffer, std::size_t nbytes,
               std::size_t offset) override {
    set_file_pointer(offset);

    ssize_t bytes_read = ::read(fd_, out_buffer, nbytes);

    if (bytes_read == -1) {
      std::string error = format("Could not read from file \"%s\": %s",
                                 path.c_str(), strerror(errno));
      throw IOException(error);
    }

    return bytes_read;
  }

  ssize_t write(const void* in_buffer, std::size_t nbytes,
                std::size_t offset) override {
    set_file_pointer(offset);

    ssize_t bytes_written = ::write(fd_, in_buffer, nbytes);

    if (bytes_written == -1) {
      std::string error = format("Could not write file \"%s\": %s",
                                 path.c_str(), strerror(errno));
      throw IOException(error);
    }

    return bytes_written;
  }

  void close() override {
    if (flocked_) {
      flock(fd_, LOCK_UN);
    }

    if (fd_ != -1) {
      ::close(fd_);
    }
  }

  Status fdatasync() override {
    int res = ::fdatasync(fd_);

    if (res != 0) {
      std::string error = format("fdatasync: %s", strerror(errno));
      return {kStatusErr, error};
    }

    return {};
  }

  Status flock(int operation, double timeout_s) override {
    Timer timer(timeout_s);

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
      if (timeout_s > 0 && timer.is_timeout()) {
        return {StatusType::kStatusErr, "flock timeout"};
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    flocked_ = true;

    return {};
  }

  int fd() const { return fd_; }

 private:
  // Set the file pointer of a file handle to a specified location.
  // Reads and writes will happen from this location.
  void set_file_pointer(std::size_t offset) const {
    off_t result = lseek(fd_, offset, SEEK_SET);

    if (result == static_cast<off_t>(-1)) {
      std::string error =
          format("Could not seek to location %lld for file \"%s\": %s", offset,
                 path.c_str(), strerror(errno));

      throw IOException(error);
    }
  }

  int fd_;
  bool flocked_;
};

std::unique_ptr<FileHandle> FileSystem::create(const char* path) noexcept {
  return open(path, O_RDWR | O_CREAT | O_TRUNC, 0666);
}

std::unique_ptr<FileHandle> FileSystem::open(const char* path, int oflag,
                                             int permission) noexcept {
  int fd = ::open(path, oflag, permission);

  if (fd == -1) {
    return nullptr;
  }

  return std::make_unique<UnixFileHandle>(path, fd);
}

bool FileSystem::exists(FileHandle& handle) {
  std::string path = handle.path;

  return access(path.c_str(), F_OK) == 0;
}

Status FileSystem::remove(FileHandle& handle) {
  std::string path = handle.path;

  int res = std::remove(path.c_str());

  if (res == -1) {
    return {StatusType::kStatusErr, strerror(res)};
  }

  return Status{};
}

std::uintmax_t FileSystem::file_size(FileHandle& handle) {
  struct stat st;
  int fd = static_cast<UnixFileHandle&>(handle).fd();
  int res = fstat(fd, &st);

  if (res == -1) {
    return static_cast<std::uintmax_t>(-1);
  }

  return st.st_size;
}

}  // namespace boltdb