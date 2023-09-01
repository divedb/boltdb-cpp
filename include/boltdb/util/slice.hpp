#ifndef BOLTDB_CPP_UTIL_SLICE_H_
#define BOLTDB_CPP_UTIL_SLICE_H_

#include <assert.h>

#include <cstddef>
#include <iterator>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

#include "boltdb/alloc/memory_pool.hpp"
#include "boltdb/util/common.hpp"
#include "boltdb/util/ref_count.hpp"
#include "boltdb/util/util.hpp"

namespace boltdb {

// The ByteSlice class represents a slice of bytes in memory. It is important to
// note that this class is not thread-safe. It internally maintains a reference
// count, which increments when it is copied or assigned.
class ByteSlice {
 public:
  using ValueType = Byte;
  using Pointer = ValueType*;
  using DiffType = typename std::iterator_traits<Pointer>::difference_type;

  static MemoryPool& pool_;

  // Construct an empty slice.
  ByteSlice() = default;

  // Construct the slice from the given data.
  // Note that, this supports construction from the binary data.
  ByteSlice(const std::vector<Byte>& bin_data);

  // Construct `ByteSlice` from the given data.
  // Note that the `data` must be \0 terminated.
  explicit ByteSlice(const Byte* cstring);
  explicit ByteSlice(const std::string& data);

  // Construct the slice with `n` copies of byte `v`.
  ByteSlice(std::size_t n, Byte v = 0);

  template <typename InputIter,
            typename = std::enable_if_t<std::is_same_v<
                Byte, typename std::iterator_traits<InputIter>::value_type>>>
  ByteSlice(InputIter first, InputIter last) {
    auto size = std::distance(first, last);

    // Add 1 extra \0 to terminate.
    auto alloc_size = round_up_to_power_of_two(size + 1);
    head_ = base_ = pool_.allocate(alloc_size);
    tail_ = std::next(head_, static_cast<DiffType>(size));
    cap_ = std::next(base_, static_cast<DiffType>(alloc_size));
    std::copy(first, last, base_);
  }

  ~ByteSlice();

  // Copy constructor and assignment.
  ByteSlice(const ByteSlice& other);
  ByteSlice& operator=(const ByteSlice& other);

  // Move constructor and assignment.
  ByteSlice(ByteSlice&& other) noexcept;
  ByteSlice& operator=(ByteSlice&& other) noexcept;

  // Append a byte.
  ByteSlice& append(Byte v);

  // Get the number of reference to same byte slice.
  int ref_count() const { return ref_count_.count(); }

  std::size_t size() const { return std::distance(head_, tail_); }

  // Get a string representation of this byte slice.
  std::string to_string() const;

  // Get a hex string representation of this byte slice.
  std::string to_hex() const;

  // Index access.
  Byte operator[](std::size_t index) const {
    assert(index < size());

    return head_[index];
  }

  Byte& operator[](std::size_t index) {
    assert(index < size());

    return head_[index];
  }

  // Remove [0, index) range from this slice.
  void remove_prefix(std::size_t index) {
    assert(index <= size());

    head_ = std::next(head_, static_cast<DiffType>(index));
  }

  std::span<Byte> span() const { return {head_, tail_}; }

  // Get a readable only view on this slice.
  const Byte* data() const { return head_; }

  std::size_t cap() const { return std::distance(base_, cap_); }

 private:
  void clear();
  void try_deallocate();
  void copy_from(const ByteSlice& other) noexcept;
  void move_from(ByteSlice&& other) noexcept;
  void grow(std::size_t new_cap);

  Byte* base_{nullptr};
  Byte* head_{nullptr};
  Byte* tail_{nullptr};
  Byte* cap_{nullptr};
  RefCount ref_count_;
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_UTIL_SLICE_H_
