#include "boltdb/util/binary.hpp"

#include <assert.h>

namespace boltdb::binary {

template <typename T>
requires std::is_integral_v<T> T BigEndian::uint(ByteSlice slice) {
  assert(slice.size() >= sizeof(T));

  if constexpr (std::is_same_v<T, u8>) {
    return slice[0];
  }

  if constexpr (std::is_same_v<T, u16>) {
    return static_cast<T>(slice[1]) | static_cast<T>(slice[0]);
  }
}

template <typename T>
requires std::is_integral_v<T>
void BigEndian::append_uint(ByteSlice slice, T v) {
  if constexpr (std::is_same_v<T, u8>) {
    slice.append(v);
  }

  if constexpr (std::is_same_v<T, u16>) {
    slice.append(static_cast<Byte>(v >> 8));
    slice.append(static_cast<Byte>(v));
  }

  if constexpr (std::is_same_v<T, u32>) {
    slice.append(static_cast<Byte>(v >> 24));
    slice.append(static_cast<Byte>(v >> 16));
    slice.append(static_cast<Byte>(v >> 8));
    slice.append(static_cast<Byte>(v));
  }

  if constexpr (std::is_same_v<T, u64>) {
    slice.append(static_cast<Byte>(v >> 56));
    slice.append(static_cast<Byte>(v >> 48));
    slice.append(static_cast<Byte>(v >> 40));
    slice.append(static_cast<Byte>(v >> 32));
    slice.append(static_cast<Byte>(v >> 24));
    slice.append(static_cast<Byte>(v >> 16));
    slice.append(static_cast<Byte>(v >> 8));
    slice.append(static_cast<Byte>(v));
  }
}

}  // namespace boltdb::binary