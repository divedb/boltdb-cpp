#ifndef BOLTDB_CPP_UTIL_BINARY_HPP_
#define BOLTDB_CPP_UTIL_BINARY_HPP_

#include <span>
#include <type_traits>

#include "boltdb/util/slice.hpp"
#include "boltdb/util/types.hpp"

namespace boltdb::binary {

// Adapt from Go.
class BigEndian {
 public:
  template <typename T>
  requires std::is_integral_v<T>
  static std::make_unsigned_t<T> uint(ByteSlice slice) {
    return uint<T>(slice.span());
  }

  // TODO(gc): makes API not consistent, any better way to make `ByteSlice` to
  // consume bytes after `uint` call.
  template <typename T>
  requires std::is_integral_v<T>
  static std::make_unsigned_t<T> uint(std::span<Byte> data) {
    using UnsignedT = std::make_unsigned_t<T>;

    if constexpr (std::is_same_v<T, i8> || std::is_same_v<T, u8>) {
      return static_cast<UnsignedT>(data[0]);
    }

    if constexpr (std::is_same_v<T, i16> || std::is_same_v<T, u16>) {
      return static_cast<UnsignedT>(data[1]) |
             (static_cast<UnsignedT>(data[0]) << 8);
    }

    if constexpr (std::is_same_v<T, i32> || std::is_same_v<T, u32>) {
      return static_cast<UnsignedT>(data[3]) |
             (static_cast<UnsignedT>(data[2]) << 8) |
             (static_cast<UnsignedT>(data[1]) << 16) |
             (static_cast<UnsignedT>(data[0]) << 24);
    }

    if constexpr (std::is_same_v<T, i64> || std::is_same_v<T, u64>) {
      return static_cast<UnsignedT>(data[7]) |
             (static_cast<UnsignedT>(data[6]) << 8) |
             (static_cast<UnsignedT>(data[5]) << 16) |
             (static_cast<UnsignedT>(data[4]) << 24) | 
             (static_cast<UnsignedT>(data[3]) << 32) | 
             (static_cast<UnsignedT>(data[2]) << 40) | 
             (static_cast<UnsignedT>(data[1]) << 48) | 
             (static_cast<UnsignedT>(data[0]) << 56);
    }
  }

  template <typename T>
  requires std::is_integral_v<T>
  static void put_uint(ByteSlice slice, T v) {
    assert(slice.size() >= sizeof(T));

    if constexpr (std::is_same_v<T, i8> || std::is_same_v<T, u8>) {
      slice[0] = static_cast<Byte>(v);
    }

    if constexpr (std::is_same_v<T, i16> || std::is_same_v<T, u16>) {
      slice[0] = static_cast<Byte>(v >> 8);
      slice[1] = static_cast<Byte>(v);
    }

    if constexpr (std::is_same_v<T, i32> || std::is_same_v<T, u32>) {
      slice[0] = static_cast<Byte>(v >> 24);
      slice[1] = static_cast<Byte>(v >> 16);
      slice[2] = static_cast<Byte>(v >> 8);
      slice[3] = static_cast<Byte>(v);
    }

    if constexpr (std::is_same_v<T, i64> || std::is_same_v<T, u64>) {
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
  static void append_uint(ByteSlice slice, T v) {
    if constexpr (std::is_same_v<T, i8> || std::is_same_v<T, u8>) {
      slice.append(static_cast<Byte>(v));
    }

    if constexpr (std::is_same_v<T, i16> || std::is_same_v<T, u16>) {
      slice.append(static_cast<Byte>(v >> 16)).append(static_cast<Byte>(v));
    }

    if constexpr (std::is_same_v<T, i32> || std::is_same_v<T, u32>) {
      slice.append(static_cast<Byte>(v >> 24))
          .append(static_cast<Byte>(v >> 16))
          .append(static_cast<Byte>(v >> 8))
          .append(static_cast<Byte>(v));
    }

    if constexpr (std::is_same_v<T, i64> || std::is_same_v<T, u64>) {
      slice.append(static_cast<Byte>(v >> 56))
          .append(static_cast<Byte>(v >> 48))
          .append(static_cast<Byte>(v >> 40))
          .append(static_cast<Byte>(v >> 32))
          .append(static_cast<Byte>(v >> 24))
          .append(static_cast<Byte>(v >> 16))
          .append(static_cast<Byte>(v >> 8))
          .append(static_cast<Byte>(v));
    }
  }
};

}  // namespace boltdb::binary

#endif  // BOLTDB_CPP_UTIL_BINARY_HPP_
