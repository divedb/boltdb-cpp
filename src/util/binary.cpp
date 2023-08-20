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
void BigEndian::put_uint(ByteSlice slice, T v) {
  assert(slice.size() >= sizeof(T));

  if constexpr (std::is_same_v<T, u8>) {
    slice[0] = v;
  }

  if constexpr (std::is_same_v<T, u16>) {
    slice[0] = Byte(v >> 8);
    slice[1] = Byte(v);
  }
}

template <typename T>
requires std::is_integral_v<T>
static void BigEndian::append_uint(ByteSlice slice, T v);

}  // namespace boltdb::binary