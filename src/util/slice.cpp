#include "boltdb/util/slice.hpp"

#include <algorithm>
#include <cstring>
#include <iomanip>
#include <sstream>

#include "boltdb/util/util.hpp"

namespace boltdb {

ByteSlice::ByteSlice(const Byte* data, std::size_t size) {
  assert(data != nullptr);

  // Add 1 extra \0 to terminate.
  size_ = size;
  cap_ = round_up_to_power_of_two(size_ + 1);
  data_ = pool_.allocate(cap_);

  std::memcpy(data_, data, size_);
}

ByteSlice::ByteSlice(std::size_t n, Byte v) : ByteSlice(std::string(n, v)) {}

ByteSlice::ByteSlice(const Byte* data) : ByteSlice(data, strlen(data)) {}

ByteSlice::ByteSlice(const std::string& data)
    : ByteSlice(data.c_str(), data.size()) {}

ByteSlice::~ByteSlice() { destroy(); }

ByteSlice& ByteSlice::operator=(const ByteSlice& other) {
  if (this == &other) {
    return *this;
  }

  destroy();
  copy(other);

  return *this;
}

ByteSlice& ByteSlice::append(Byte v) {
  if (size_ >= cap_) {
    grow();
  }

  data_[size_] = v;
  size_++;

  return *this;
}

std::string ByteSlice::to_string() const { return {data_, size_}; }

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
    hex += format("0x%02hhx", data_[0]);

    for (std::size_t i = 1; i < size_; i++) {
      hex += "," + format("0x%02hhx", data_[i]);
    }
  }

  hex += "]";

  return hex;
}

// Decrease the number of reference by 1.
// Free the memory if this is the last observer.
void ByteSlice::destroy() {
  if (data_ != nullptr && ref_count_.unique()) {
    pool_.deallocate(data_, cap_);
  }
}

void ByteSlice::copy(const ByteSlice& other) {
  data_ = other.data_;
  size_ = other.size_;
  cap_ = other.cap_;
  ref_count_ = other.ref_count_;
}

void ByteSlice::grow() {
  std::size_t new_cap = std::max(1UL, cap_ * 2);
  Byte* new_data = pool_.allocate(new_cap);

  // Binary data may contain \0.
  // Hence strcpy may not work.
  memcpy(new_data, data_, size_);

  // Reset the reference count since slice point to new memory.
  destroy();
  ref_count_.reset();

  data_ = new_data;
  cap_ = new_cap;
}

}  // namespace boltdb