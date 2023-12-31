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
    using UByte = std::make_unsigned_t<Byte>;
    using UnsignedT = std::make_unsigned_t<T>;

    if constexpr (sizeof(T) == sizeof(u8)) {
      return static_cast<UnsignedT>(static_cast<UByte>(slice[0]));
    }

    if constexpr (sizeof(T) == sizeof(u16)) {
      return static_cast<UnsignedT>(static_cast<UByte>(slice[1])) |
             (static_cast<UnsignedT>(static_cast<UByte>(slice[0])) << 8);
    }

    if constexpr (sizeof(T) == sizeof(u32)) {
      return static_cast<UnsignedT>(static_cast<UByte>(slice[3])) |
             (static_cast<UnsignedT>(static_cast<UByte>(slice[2])) << 8) |
             (static_cast<UnsignedT>(static_cast<UByte>(slice[1])) << 16) |
             (static_cast<UnsignedT>(static_cast<UByte>(slice[0])) << 24);
    }

    if constexpr (sizeof(T) == sizeof(u64)) {
      return static_cast<UnsignedT>(static_cast<UByte>(slice[7])) |
             (static_cast<UnsignedT>(static_cast<UByte>(slice[6])) << 8) |
             (static_cast<UnsignedT>(static_cast<UByte>(slice[5])) << 16) |
             (static_cast<UnsignedT>(static_cast<UByte>(slice[4])) << 24) |
             (static_cast<UnsignedT>(static_cast<UByte>(slice[3])) << 32) |
             (static_cast<UnsignedT>(static_cast<UByte>(slice[2])) << 40) |
             (static_cast<UnsignedT>(static_cast<UByte>(slice[1])) << 48) |
             (static_cast<UnsignedT>(static_cast<UByte>(slice[0])) << 56);
    }
  }

  template <typename T>
    requires std::is_integral_v<T>
  static void put_uint(ByteSlice slice, T v) {
    assert(slice.size() >= sizeof(T));

    if constexpr (sizeof(T) == sizeof(u8)) {
      slice[0] = static_cast<Byte>(v);
    }

    if constexpr (sizeof(T) == sizeof(u16)) {
      slice[0] = static_cast<Byte>(v >> 8);
      slice[1] = static_cast<Byte>(v);
    }

    if constexpr (sizeof(T) == sizeof(u32)) {
      slice[0] = static_cast<Byte>(v >> 24);
      slice[1] = static_cast<Byte>(v >> 16);
      slice[2] = static_cast<Byte>(v >> 8);
      slice[3] = static_cast<Byte>(v);
    }

    if constexpr (sizeof(T) == sizeof(u64)) {
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
  static ByteSlice append_uint(ByteSlice slice, T v) {
    if constexpr (sizeof(T) == sizeof(u8)) {
      return slice.append(static_cast<Byte>(v));
    }

    if constexpr (sizeof(T) == sizeof(u16)) {
      return slice.append(static_cast<Byte>(v >> 8))
          .append(static_cast<Byte>(v));
    }

    if constexpr (sizeof(T) == sizeof(u32)) {
      return slice.append(static_cast<Byte>(v >> 24))
          .append(static_cast<Byte>(v >> 16))
          .append(static_cast<Byte>(v >> 8))
          .append(static_cast<Byte>(v));
    }

    if constexpr (sizeof(T) == sizeof(u64)) {
      return slice.append(static_cast<Byte>(v >> 56))
          .append(static_cast<Byte>(v >> 48))
          .append(static_cast<Byte>(v >> 40))
          .append(static_cast<Byte>(v >> 32))
          .append(static_cast<Byte>(v >> 24))
          .append(static_cast<Byte>(v >> 16))
          .append(static_cast<Byte>(v >> 8))
          .append(static_cast<Byte>(v));
    }

    return slice;
  }

  template <typename... Integers>
    requires std::is_integral_v<std::common_type_t<Integers...>>
  static ByteSlice append_variadic_uint(ByteSlice slice, Integers... integers) {
    (..., (slice = append_uint(slice, integers)));
    return slice;
  }
};

class LittleEndian {
 public:
  template <typename T>
    requires std::is_integral_v<T>
  static std::make_unsigned_t<T> uint(ByteSlice slice) {
    using UByte = std::make_unsigned_t<Byte>;
    using UnsignedT = std::make_unsigned_t<T>;

    if constexpr (sizeof(T) == sizeof(u8)) {
      return static_cast<UnsignedT>(static_cast<UByte>(slice[0]));
    }

    if constexpr (sizeof(T) == sizeof(u16)) {
      return static_cast<UnsignedT>(static_cast<UByte>(slice[0])) |
             (static_cast<UnsignedT>(static_cast<UByte>(slice[1])) << 8);
    }

    if constexpr (sizeof(T) == sizeof(u32)) {
      return static_cast<UnsignedT>(static_cast<UByte>(slice[0])) |
             (static_cast<UnsignedT>(static_cast<UByte>(slice[1])) << 8) |
             (static_cast<UnsignedT>(static_cast<UByte>(slice[2])) << 16) |
             (static_cast<UnsignedT>(static_cast<UByte>(slice[3])) << 24);
    }

    if constexpr (sizeof(T) == sizeof(u64)) {
      return static_cast<UnsignedT>(static_cast<UByte>(slice[0])) |
             (static_cast<UnsignedT>(static_cast<UByte>(slice[1])) << 8) |
             (static_cast<UnsignedT>(static_cast<UByte>(slice[2])) << 16) |
             (static_cast<UnsignedT>(static_cast<UByte>(slice[3])) << 24) |
             (static_cast<UnsignedT>(static_cast<UByte>(slice[4])) << 32) |
             (static_cast<UnsignedT>(static_cast<UByte>(slice[5])) << 40) |
             (static_cast<UnsignedT>(static_cast<UByte>(slice[6])) << 48) |
             (static_cast<UnsignedT>(static_cast<UByte>(slice[7])) << 56);
    }
  }

  template <typename T>
    requires std::is_integral_v<T>
  static void put_uint(ByteSlice slice, T v) {
    assert(slice.size() >= sizeof(T));

    if constexpr (sizeof(T) == sizeof(u8)) {
      slice[0] = static_cast<Byte>(v);
    }

    if constexpr (sizeof(T) == sizeof(u16)) {
      slice[1] = static_cast<Byte>(v >> 8);
      slice[0] = static_cast<Byte>(v);
    }

    if constexpr (sizeof(T) == sizeof(u32)) {
      slice[3] = static_cast<Byte>(v >> 24);
      slice[2] = static_cast<Byte>(v >> 16);
      slice[1] = static_cast<Byte>(v >> 8);
      slice[0] = static_cast<Byte>(v);
    }

    if constexpr (sizeof(T) == sizeof(u64)) {
      slice[7] = static_cast<Byte>(v >> 56);
      slice[6] = static_cast<Byte>(v >> 48);
      slice[5] = static_cast<Byte>(v >> 40);
      slice[4] = static_cast<Byte>(v >> 32);
      slice[3] = static_cast<Byte>(v >> 24);
      slice[2] = static_cast<Byte>(v >> 16);
      slice[1] = static_cast<Byte>(v >> 8);
      slice[0] = static_cast<Byte>(v);
    }
  }

  template <typename T>
    requires std::is_integral_v<T>
  static ByteSlice append_uint(ByteSlice slice, T v) {
    if constexpr (sizeof(T) == sizeof(u8)) {
      return slice.append(static_cast<Byte>(v));
    }

    if constexpr (sizeof(T) == sizeof(u16)) {
      return slice.append(static_cast<Byte>(v))
          .append(static_cast<Byte>(v >> 8));
    }

    if constexpr (sizeof(T) == sizeof(u32)) {
      return slice.append(static_cast<Byte>(v))
          .append(static_cast<Byte>(v >> 8))
          .append(static_cast<Byte>(v >> 16))
          .append(static_cast<Byte>(v >> 24));
    }

    if constexpr (sizeof(T) == sizeof(u64)) {
      return slice.append(static_cast<Byte>(v))
          .append(static_cast<Byte>(v >> 8))
          .append(static_cast<Byte>(v >> 16))
          .append(static_cast<Byte>(v >> 24))
          .append(static_cast<Byte>(v >> 32))
          .append(static_cast<Byte>(v >> 40))
          .append(static_cast<Byte>(v >> 48))
          .append(static_cast<Byte>(v >> 56));
    }

    return slice;
  }

  template <typename... Integers>
    requires std::is_integral_v<std::common_type_t<Integers...>>
  static ByteSlice append_variadic_uint(ByteSlice slice, Integers... integers) {
    (..., (slice = append_uint(slice, integers)));
    return slice;
  }
};

}  // namespace boltdb::binary

#endif  // BOLTDB_CPP_UTIL_BINARY_HPP_
