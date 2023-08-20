#include "boltdb/util/slice.hpp"

#include <algorithm>
#include <cstring>
#include <iomanip>
#include <sstream>

#include "boltdb/util/util.hpp"

namespace boltdb {

ByteSlice::ByteSlice() : data_(nullptr), size_(0), cap_(0) {}

ByteSlice::ByteSlice(const Byte* data, std::size_t size) {
  // TODO(gc): validate argument (NULL)
  auto& pool = MemoryPool::instance();

  // Add extra 1 to termainate.
  size_ = std::min(strlen(data), size);
  cap_ = round_up_to_power_of_two(size_ + 1);
  data_ = pool.allocate(cap_);

  std::strncpy(data_, data, size);
}

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
}

std::string ByteSlice::to_string() const { return {data_, size_}; }

std::string ByteSlice::to_hex() const {
  if (size() == 0) {
    return "[]";
  }

  std::ostringstream oss;
  oss << '[';
  oss << std::hex << data_[0];

  for (std::size_t i = 1; i < size_; i++) {
    oss << ',' << std::hex << data_[i];
  }

  oss << ']';

  return oss.str();
}

// TODO(gc): release the reference count here.
void ByteSlice::destroy() {
  if (ref_count_.unique() && data_) {
    MemoryPool::instance().deallocate(data_, cap_);
    ref_count_.release();
  }
}

void ByteSlice::copy(const ByteSlice& other) {
  data_ = other.data_;
  size_ = other.size_;
  cap_ = other.cap_;
  ref_count_ = other.ref_count_;
}

void ByteSlice::grow() {
  std::size_t new_cap = std::max(1, cap_ * 2);

  Byte* new_data = MemoryPool::instance().allocate(new_cap);
  strncpy(new_data, data_, size_);
  destroy();

  data_ = new_data;
  cap_ = new_cap_;
}

}  // namespace boltdb