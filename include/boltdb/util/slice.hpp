#ifndef BOLTDB_CPP_UTIL_SLICE_H_
#define BOLTDB_CPP_UTIL_SLICE_H_

#include <assert.h>

#include <algorithm>
#include <cstddef>
#include <span>
#include <string>

#include "boltdb/alloc/memory_pool.hpp"
#include "boltdb/util/common.hpp"
#include "boltdb/util/ref_count.hpp"

namespace boltdb {

// The ByteSlice class represents a slice of bytes in memory. It is important to
// note that this class is not thread-safe. It internally maintains a reference
// count, which increments when it is copied or assigned.
class ByteSlice {
 public:
  static MemoryPool& pool_;

  // Construct an empty slice.
  ByteSlice() = default;

  // Construct the slice from the given data.
  // Note that, this supports construction from the binary data provided that
  // the `size` is valid. That is, dereference *(bin_data + size) is valid.
  ByteSlice(const Byte* bin_data, std::size_t size);

  // Construct `ByteSlice` from the given data.
  // Note that the `data` must be \0 terminated.
  explicit ByteSlice(const Byte* cstring);
  explicit ByteSlice(const std::string& data);

  // Construct the slice with `n` copies of byte `v`.
  ByteSlice(std::size_t n, Byte v = 0);

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

  std::size_t size() const { return size_; }

  // Get a string representation of this byte slice.
  std::string to_string() const;

  // Get a hex string representation of this byte slice.
  std::string to_hex() const;

  // Index access.
  Byte operator[](int index) const {
    assert(static_cast<std::size_t>(index) < size_);

    return cursor_[index];
  }

  Byte& operator[](int index) {
    assert(static_cast<std::size_t>(index) < size_);

    return cursor_[index];
  }

  // Advance n steps.
  void advance(std::size_t n) {
    assert(static_cast<std::size_t>(n) < size_);
    size_ -= n;
    std::advance(cursor_, n);
  }

  std::span<Byte> span() const { return {cursor_, size_}; }

  // Get a readable only view on this slice.
  const Byte* data() const { return cursor_; }

 private:
  void clear();
  void try_deallocate();
  void copy_from(const ByteSlice& other) noexcept;
  void move_from(ByteSlice&& other) noexcept;
  void grow();

  Byte* data_{nullptr};
  Byte* cursor_{nullptr};
  std::size_t size_{0};
  std::size_t cap_{0};
  RefCount ref_count_;
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_UTIL_SLICE_H_
