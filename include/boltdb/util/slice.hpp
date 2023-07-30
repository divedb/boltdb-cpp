#ifndef BOLTDB_CPP_UTIL_SLICE_H_
#define BOLTDB_CPP_UTIL_SLICE_H_

#include <cstddef>
#include <string>

#include "boltdb/alloc/memory_pool.hpp"
#include "boltdb/util/common.hpp"
#include "boltdb/util/ref_count.hpp"

namespace boltdb {

class ByteSlice {
 public:
  ByteSlice();
  ByteSlice(const Byte* data, std::size_t size);
  explicit ByteSlice(const Byte* data);
  explicit ByteSlice(const std::string& data);
  ByteSlice(const ByteSlice& other);

  ~ByteSlice();

  ByteSlice& operator=(const ByteSlice& other);

  // Get the number of reference to same byte slice.
  int ref_count() const { return _ref_count.count(); }

  std::size_t size() const { return _size; }

  // Get a string representation of this byte slice.
  std::string to_string() const { return std::string(_data, _size); }

 private:
  void destroy();
  void copy(const ByteSlice& other);

  Byte* _data;
  std::size_t _size;
  std::size_t _cap;
  RefCount _ref_count;
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_UTIL_SLICE_H_
