#ifndef BOLTDB_CPP_UTIL_COMMON_HPP_
#define BOLTDB_CPP_UTIL_COMMON_HPP_

namespace boltdb {

#define DISALLOW_COPY(Typename) \
  Typename(const Typename&);    \
  void operator=(const Typename&)

#define DISALLOW_MOVE(Typename) \
  Typename(const Typename&&);   \
  void operator=(const Typename&&)

#define UNUSED_VAR(var) ((void)(var))

}  // namespace boltdb

#endif  // BOLTDB_CPP_UTIL_COMMON_HPP_
