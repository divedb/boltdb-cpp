#ifndef BOLTDB_CPP_UTIL_COMMON_HPP_
#define BOLTDB_CPP_UTIL_COMMON_HPP_

#include <string>

namespace boltdb {

#define NO_DISCARD [[nodiscard]]

#define DISALLOW_COPY(Typename) \
  Typename(const Typename&);    \
  void operator=(const Typename&)

#define DISALLOW_MOVE(Typename)        \
  Typename(const Typename&&) noexcept; \
  void operator=(const Typename&&) noexcept

#define UNUSED_VAR(var) ((void)(var))

static const std::string kAsciiLowercase = "abcdefghijklmnopqrstuvwxyz";
static const std::string kAsciiUppercase = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const std::string kOctDidigits = "01234567";
static const std::string kWhitSpace = " \t\n\r\x0b\x0c";
static const std::string kHexDigits = "0123456789abcdefABCDEF";

}  // namespace boltdb

#endif  // BOLTDB_CPP_UTIL_COMMON_HPP_
