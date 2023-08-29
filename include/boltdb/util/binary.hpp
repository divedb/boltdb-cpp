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
  requires std::is_integral_v<T> static std::make_unsigned_t<T> uint(
      ByteSlice slice) {
    return uint<T>(slice.span());
  }

  // TODO(gc): makes API not consistent, any better way to make `ByteSlice` to
  // consume bytes after `uint` call.
  template <typename T>
  requires std::is_integral_v<T> static std::make_unsigned_t<T> uint(
      std::span<Byte> data) {
    using UnsignedT = std::make_unsigned_t<T>;
    using UByte = std::make_unsigned_t<Byte>;

    if constexpr (std::is_same_v<T, i8> || std::is_same_v<T, u8>) {
      return static_cast<UnsignedT>(static_cast<UByte>(data[0]));
    }

    if constexpr (std::is_same_v<T, i16> || std::is_same_v<T, u16>) {
      return static_cast<UnsignedT>(static_cast<UByte>(data[1])) |
             (static_cast<UnsignedT>(static_cast<UByte>(data[0])) << 8);
    }

    if constexpr (std::is_same_v<T, i32> || std::is_same_v<T, u32>) {
      return static_cast<UnsignedT>(static_cast<UByte>(data[3])) |
             (static_cast<UnsignedT>(static_cast<UByte>(data[2])) << 8) |
             (static_cast<UnsignedT>(static_cast<UByte>(data[1])) << 16) |
             (static_cast<UnsignedT>(static_cast<UByte>(data[0])) << 24);
    }

    if constexpr (std::is_same_v<T, i64> || std::is_same_v<T, u64>) {
      return static_cast<UnsignedT>(static_cast<UByte>(data[7])) |
             (static_cast<UnsignedT>(static_cast<UByte>(data[6])) << 8) |
             (static_cast<UnsignedT>(static_cast<UByte>(data[5])) << 16) |
             (static_cast<UnsignedT>(static_cast<UByte>(data[4])) << 24) |
             (static_cast<UnsignedT>(static_cast<UByte>(data[3])) << 32) |
             (static_cast<UnsignedT>(static_cast<UByte>(data[2])) << 40) |
             (static_cast<UnsignedT>(static_cast<UByte>(data[1])) << 48) |
             (static_cast<UnsignedT>(static_cast<UByte>(data[0])) << 56);
    }
  }

  template <typename T>
  requires std::is_integral_v<T> static void put_uint(std::span<Byte> data,
                                                      T v) {
    assert(data.size() >= sizeof(T));

    using UnsignedT = std::make_unsigned_t<T>;

    if constexpr (std::is_same_v<UnsignedT, u8>) {
      data[0] = static_cast<Byte>(v);
    }

    if constexpr (std::is_same_v<UnsignedT, u16>) {
      data[0] = static_cast<Byte>(v >> 8);
      data[1] = static_cast<Byte>(v);
    }

    if constexpr (std::is_same_v<UnsignedT, u32>) {
      data[0] = static_cast<Byte>(v >> 24);
      data[1] = static_cast<Byte>(v >> 16);
      data[2] = static_cast<Byte>(v >> 8);
      data[3] = static_cast<Byte>(v);
    }

    if constexpr (std::is_same_v<UnsignedT, u64>) {
      data[0] = static_cast<Byte>(v >> 56);
      data[1] = static_cast<Byte>(v >> 48);
      data[2] = static_cast<Byte>(v >> 40);
      data[3] = static_cast<Byte>(v >> 32);
      data[4] = static_cast<Byte>(v >> 24);
      data[5] = static_cast<Byte>(v >> 16);
      data[6] = static_cast<Byte>(v >> 8);
      data[7] = static_cast<Byte>(v);
    }
  }

  template <typename T>
  requires std::is_integral_v<T> static void put_uint(ByteSlice slice, T v) {
    put_uint(slice.span(), v);
  }

  template <typename T>
  requires std::is_integral_v<T> static ByteSlice append_uint(ByteSlice slice,
                                                              T v) {
    if constexpr (std::is_same_v<T, i8> || std::is_same_v<T, u8>) {
      return slice.append(static_cast<Byte>(v));
    }

    if constexpr (std::is_same_v<T, i16> || std::is_same_v<T, u16>) {
      return slice.append(static_cast<Byte>(v >> 16))
          .append(static_cast<Byte>(v));
    }

    if constexpr (std::is_same_v<T, i32> || std::is_same_v<T, u32>) {
      return slice.append(static_cast<Byte>(v >> 24))
          .append(static_cast<Byte>(v >> 16))
          .append(static_cast<Byte>(v >> 8))
          .append(static_cast<Byte>(v));
    }

    if constexpr (std::is_same_v<T, i64> || std::is_same_v<T, u64>) {
      return slice.append(static_cast<Byte>(v >> 56))
          .append(static_cast<Byte>(v >> 48))
          .append(static_cast<Byte>(v >> 40))
          .append(static_cast<Byte>(v >> 32))
          .append(static_cast<Byte>(v >> 24))
          .append(static_cast<Byte>(v >> 16))
          .append(static_cast<Byte>(v >> 8))
          .append(static_cast<Byte>(v));
    }
  }

  //   template <typename... Integers>
  //   requires std::conjunction_v<std::is_same_v<Integers>...>
  //   static ByteSlice append_variadic_uint(ByteSlice slice, Integers...
  //   integers) {
  //     (..., (slice = append_uint(slice, integers)));
  //     return slice;
  //   }
};

}  // namespace boltdb::binary

#endif  // BOLTDB_CPP_UTIL_BINARY_HPP_
