#ifndef BOLTDB_CPP_UTIL_CONST_HPP_
#define BOLTDB_CPP_UTIL_CONST_HPP_

#include "boltdb/util/types.hpp"

namespace boltdb {

constexpr static const PageID kInvalidPageID = 0xFFFFFFFFFFFFFFFF;

// Avaiable page id. The first two pages are used for meta information.
constexpr static const PageID kAvailPageID = 0x02;

}  // namespace boltdb

#endif  // BOLTDB_CPP_UTIL_CONST_HPP_
