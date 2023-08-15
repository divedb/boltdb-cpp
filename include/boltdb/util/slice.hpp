#ifndef BOLTDB_CPP_UTIL_SLICE_H_
#define BOLTDB_CPP_UTIL_SLICE_H_

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
  ByteSlice(const ByteSlice& other);

  ~ByteSlice();

  // Copy assignment.
  ByteSlice& operator=(const ByteSlice& other);

  // Get the number of reference to same byte slice.
  [[nodiscard]] int ref_count() const { return _ref_count.count(); }

  [[nodiscard]] std::size_t size() const { return _size; }

  // Get a string representation of this byte slice.
  [[nodiscard]] std::string to_string() const;

  // Get a hex string representation of this byte slice.
  [[nodiscard]] std::string to_hex() const;

 private:
  void destroy();
  void copy(const ByteSlice& other);

  Byte* data_;
  std::size_t size_;
  std::size_t cap_;
  RefCount ref_count_;
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_UTIL_SLICE_H_
