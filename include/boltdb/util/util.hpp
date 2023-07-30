#ifndef BOLTDB_CPP_UTIL_HPP_
#define BOLTDB_CPP_UTIL_HPP_

#include <cassert>
#include <cstdarg>
#include <cstdlib>
#include <string>

namespace boltdb {

const static int kPageSize = 4096;

// Round up the given `x` to nearest power of two.
// Note that `x` must be positive and type of int
// is appropriate to cover the most cases.
constexpr static inline int round_up_to_power_of_two(int x) {
  assert(x > 0);

  x--;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  x++;

  return x;
}

// A simple implementation of format.
std::string format(const char* fmt, ...) {
  static char buf[1024];

  va_list args;
  va_start(args, fmt);
  std::vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  return std::string(buf);
}

}  // namespace boltdb

#endif  // BOLTDB_CPP_UTIL_H_
