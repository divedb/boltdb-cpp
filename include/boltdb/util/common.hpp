#ifndef BOLTDB_CPP_UTIL_COMMON_HPP_
#define BOLTDB_CPP_UTIL_COMMON_HPP_

#include <cstdint>

namespace boltdb {

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;

// TODO(gc): why not use std::byte
using Byte = char;
using PageID = u64;
using TxnID = u64;

#define DISALLOW_COPY(Typename)      \
  Typename(const Typename&) = delte; \
  void operator=(const Typename&) = d

#define DISALLOW_MOVE(Typename) \
  Typename(const Typename&&);   \
  void operator=(const Typename&&)

#define UNUSED_VAR(var) ((void)(var))

}  // namespace boltdb

#endif  // BOLTDB_CPP_UTIL_COMMON_HPP_
