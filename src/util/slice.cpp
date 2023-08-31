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
  auto alloc_size = round_up_to_power_of_two(size + 1);
  head_ = base_ = pool_.allocate(alloc_size);
  tail_ = std::next(head_, static_cast<DiffType>(size));
  cap_ = std::next(base_, static_cast<DiffType>(alloc_size));
  std::memcpy(base_, data, size);
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
  if (tail_ == cap_) {
    std::size_t new_cap = std::max(1UL, cap() * 2);
    grow(new_cap);
  }

  *tail_++ = v;

  return *this;
}

std::string ByteSlice::to_string() const { return {head_, size()}; }

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

  std::ostringstream hex;
  std::transform(head_, tail_, std::ostream_iterator<std::string>{hex},
                 [](Byte c) { return format("0x%02hhx,", c); });

  return hex.str();
}

void ByteSlice::clear() {
  base_ = nullptr;
  head_ = nullptr;
  tail_ = nullptr;
  cap_ = nullptr;
}

// Decrease the number of reference by 1.
// Free the memory if this is the last observer.
void ByteSlice::try_deallocate() {
  if (base_ != nullptr && ref_count_.unique()) {
    pool_.deallocate(base_, cap());
  }

  clear();
}

void ByteSlice::copy_from(const ByteSlice& other) noexcept {
  base_ = other.base_;
  head_ = other.head_;
  tail_ = other.tail_;
  cap_ = other.cap_;
  ref_count_ = other.ref_count_;
}

void ByteSlice::move_from(ByteSlice&& other) noexcept {
#define MOV_AUX(src, dst) \
  do {                    \
    src = dst;            \
    dst = {};             \
  } while (0)

  MOV_AUX(base_, other.base_);
  MOV_AUX(head_, other.head_);
  MOV_AUX(tail_, other.tail_);
  MOV_AUX(cap_, other.cap_);
  ref_count_ = std::move(other.ref_count_);

#undef MOV_AUX
}

void ByteSlice::grow(std::size_t new_cap) {
  auto sz = size();
  Byte* new_data = pool_.allocate(new_cap);

  // Binary data may contain \0. Hence strcpy may not work.
  memcpy(new_data, head_, sz);

  // Reset the reference count since slice point to new memory.
  try_deallocate();
  ref_count_.reset();

  base_ = new_data;
  head_ = base_;
  tail_ = std::next(head_, static_cast<DiffType>(sz));
  cap_ = std::next(head_, static_cast<DiffType>(new_cap));
}

}  // namespace boltdb