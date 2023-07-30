#ifndef BOLTDB_CPP_UTIL_COMPARATOR_HPP_
#define BOLTDB_CPP_UTIL_COMPARATOR_HPP_

#include "boltdb/util/common.hpp"

namespace boltdb {

// TODO(gc): strong_ordering?
enum Ordering : u8 { kLess = 0x01, kEqual = 0x02, kGreater = 0x04 };

// A markable class means that type `T` is comparable.
// Note that type `T` needs to implement `==` and `<` operators.
template <typename T>
class Comparable {
 public:
  Ordering compare(const T& lhs, const T& rhs) {
    if (lhs == rhs) {
      return kEqual;
    } else if (lhs < rhs) {
      return kLess;
    } else {
      return kGreater;
    }
  }
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_UTIL_COMPARATOR_HPP_