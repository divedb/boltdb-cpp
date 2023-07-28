#ifndef BOLTDB_CPP_UTIL_SLICE_H_
#define BOLTDB_CPP_UTIL_SLICE_H_

#include <cstddef>
#include <string>

#include "boltdb/alloc/memory_pool.hpp"
#include "boltdb/util/common.hpp"

namespace boltdb {

class ByteSlice {
 public:
  ByteSlice();
  ByteSlice(Byte const* data, std::size_t size);
  explicit ByteSlice(Byte const* data);
  explicit ByteSlice(std::string const& data);

  ~ByteSlice();

  // Copy constructor and assignment.
  ByteSlice(ByteSlice& other);
  ByteSlice& operator=(ByteSlice& other);

  // Move constructor and assignment.
  ByteSlice(ByteSlice&& other);
  ByteSlice& operator=(ByteSlice&& other);

  // Return true if this byte slice reference to something false otherwise.
  operator bool() const { return _ref_count != nullptr; }

  // Get the number of reference to same byte slice.
  int ref_count() const {
    if (_ref_count == nullptr) {
      return 0;
    }

    return *_ref_count;
  }

  // Get a string representation of this byte slice.
  std::string to_string() const { return std::string(_data, _size); }

 private:
  void destroy();
  void shallow_copy(ByteSlice& other);
  void reset();

  Byte* _data;
  std::size_t _size;
  std::size_t _cap;
  int* _ref_count;
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_UTIL_SLICE_H_
