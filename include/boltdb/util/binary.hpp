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
  static void put_uint(ByteSlice slice, T v) {
	  assert(slice.size() >= sizeof(T));

	  if constexpr (std::is_same_v<T, u8>) {
	    slice[0] = v;
	  }

	  if constexpr (std::is_same_v<T, u16>) {
	    slice[0] = static_cast<Byte>(v >> 8);
	    slice[1] = static_cast<Byte>(v);
	  }

	  if constexpr (std::is_same_v<T, u32>) {
	    slice[0] = static_cast<Byte>(v >> 24);
	    slice[1] = static_cast<Byte>(v >> 16);
	    slice[2] = static_cast<Byte>(v >> 8);
	    slice[3] = static_cast<Byte>(v);
	  }

	  if constexpr (std::is_same_v<T, u64>) {
	    slice[0] = static_cast<Byte>(v >> 56);
	    slice[1] = static_cast<Byte>(v >> 48);
	    slice[2] = static_cast<Byte>(v >> 40);
	    slice[3] = static_cast<Byte>(v >> 32);
	    slice[4] = static_cast<Byte>(v >> 24);
	    slice[5] = static_cast<Byte>(v >> 16);
	    slice[6] = static_cast<Byte>(v >> 8);
	    slice[7] = static_cast<Byte>(v);
	  }
	}

  template <typename T>
  requires std::is_integral_v<T>
  static void append_uint(ByteSlice slice, T v);
};

}  // namespace boltdb::binary

#endif  // BOLTDB_CPP_UTIL_BINARY_HPP_