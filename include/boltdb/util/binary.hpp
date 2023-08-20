#ifndef BOLTDB_CPP_UTIL_BINARY_HPP_
#define BOLTDB_CPP_UTIL_BINARY_HPP_

#include <type_traits>

#include "boltdb/util/slice.hpp"
#include "boltdb/util/types.hpp"

namespace boltdb::binary {

// Adapt from Go.
class BigEndian {
 public:
  template <typename T>
  requires std::is_integral_v<T>
  static T uint(ByteSlice slice);

  template <typename T>
  requires std::is_integral_v<T>
  static void put_uint(ByteSlice slice, T v);

  template <typename T>
  requires std::is_integral_v<T>
  static void append_uint(ByteSlice slice, T v);
};

}  // namespace boltdb::binary

#endif  // BOLTDB_CPP_UTIL_BINARY_HPP_