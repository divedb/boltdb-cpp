#include "boltdb/util/slice.hpp"

#include <algorithm>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <type_traits>

#include "boltdb/util/util.hpp"

namespace boltdb {

MemoryPool& ByteSlice::pool_ = MemoryPool::instance();

ByteSlice::ByteSlice(const Byte* data, std::size_t size) {
  assert(data != nullptr);

  // Add 1 extra \0 to terminate.
  size_ = size;
  cap_ = round_up_to_power_of_two(size_ + 1);
  data_ = pool_.allocate(cap_);
  cursor_ = data_;

  std::memcpy(data_, data, size_);
}

ByteSlice::ByteSlice(const Byte* cstring)
    : ByteSlice(cstring, strlen(cstring)) {}

ByteSlice::ByteSlice(const std::string& data)
    : ByteSlice(data.c_str(), data.size()) {}

ByteSlice::ByteSlice(std::size_t n, Byte v) : ByteSlice(std::string(n, v)) {}

ByteSlice::~ByteSlice() { try_deallocate(); }

ByteSlice::ByteSlice(const ByteSlice& other) : ref_count_(other.ref_count_) {
  copy_from(other);
}

ByteSlice& ByteSlice::operator=(const ByteSlice& other) {
  if (this == &other) {
    return *this;
  }

  try_deallocate();
  copy_from(other);

  return *this;
}

// TODO(gc): this incurs extra `new int` in RefCount since ref_count_ is not
// initialized in member list.
ByteSlice::ByteSlice(ByteSlice&& other) noexcept {
  move_from(std::move(other));
}

ByteSlice& ByteSlice::operator=(ByteSlice&& other) noexcept {
  try_deallocate();
  move_from(std::move(other));

  return *this;
}

ByteSlice& ByteSlice::append(Byte v) {
  if (size_ >= cap_) {
    grow();
  }

  cursor_[size_] = v;
  size_++;

  return *this;
}

std::string ByteSlice::to_string() const { return {cursor_, size_}; }

std::string ByteSlice::to_hex() const {
  //   std::ostringstream oss;
  //   oss << '[';

  //   if (size() > 0) {
  //     oss << std::hex << data_[0];

  //     for (std::size_t i = 1; i < size_; i++) {
  //       oss << ',' << std::hex << data_[i];
  //     }
  //   }

  //   oss << ']';

  //   return oss.str();

  std::string hex = "[";

  if (size() > 0) {
    hex += format("0x%02hhx", cursor_[0]);

    for (std::size_t i = 1; i < size_; i++) {
      hex += "," + format("0x%02hhx", cursor_[i]);
    }
  }

  hex += "]";

  return hex;
}

void ByteSlice::clear() {
  data_ = nullptr;
  cursor_ = nullptr;
  size_ = 0;
  cap_ = 0;
}

// Decrease the number of reference by 1.
// Free the memory if this is the last observer.
void ByteSlice::try_deallocate() {
  if (data_ != nullptr && ref_count_.unique()) {
    pool_.deallocate(data_, cap_);
  }

  clear();
}

void ByteSlice::copy_from(const ByteSlice& other) noexcept {
  data_ = other.data_;
  cursor_ = other.cursor_;
  size_ = other.size_;
  cap_ = other.cap_;
  ref_count_ = other.ref_count_;
}

void ByteSlice::move_from(ByteSlice&& other) noexcept {
#define MOV_AUX(src, dst) \
  do {                    \
    src = dst;            \
    dst = {};             \
  } while (0)

  MOV_AUX(data_, other.data_);
  MOV_AUX(cursor_, other.cursor_);
  MOV_AUX(size_, other.size_);
  MOV_AUX(cap_, other.cap_);
  ref_count_ = std::move(other.ref_count_);

#undef MOV_AUX
}

void ByteSlice::grow() {
  std::size_t new_cap = std::max(1UL, cap_ * 2);
  Byte* new_data = pool_.allocate(new_cap);
  ssize_t offset = std::distance(data_, cursor_);

  // Binary data may contain \0. Hence strcpy may not work.
  memcpy(new_data, data_, size_);

  // Reset the reference count since slice point to new memory.
  try_deallocate();
  ref_count_.reset();

  data_ = new_data;
  cursor_ = data_;
  cap_ = new_cap;
  std::advance(cursor_, offset);
}

}  // namespace boltdb