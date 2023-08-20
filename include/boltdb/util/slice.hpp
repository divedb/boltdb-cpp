#ifndef BOLTDB_CPP_UTIL_SLICE_H_
#define BOLTDB_CPP_UTIL_SLICE_H_

#include <assert.h>

#include <cstddef>
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
  ByteSlice();
  ByteSlice(const Byte* data, std::size_t size);
  explicit ByteSlice(const Byte* data);
  explicit ByteSlice(const std::string& data);
  ByteSlice(const ByteSlice& other) = default;

  ~ByteSlice();

  // Copy assignment.
  ByteSlice& operator=(const ByteSlice& other);

  // Append a byte.
  ByteSlice& append(Byte v);

  // Get the number of reference to same byte slice.
  [[nodiscard]] int ref_count() const { return ref_count_.count(); }

  [[nodiscard]] std::size_t size() const { return size_; }

  // Get a string representation of this byte slice.
  [[nodiscard]] std::string to_string() const;

  // Get a hex string representation of this byte slice.
  [[nodiscard]] std::string to_hex() const;

  Byte operator[](int index) const {
    assert(index < size_);

    return data_[index];
  }

  Byte& operator[](int index) {
    assert(index < size_);

    return data_[index];
  }

 private:
  void destroy();
  void copy(const ByteSlice& other);
  void grow();

  Byte* data_;
  std::size_t size_;
  std::size_t cap_;
  RefCount ref_count_;
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_UTIL_SLICE_H_
