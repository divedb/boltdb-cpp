#ifndef BOLTDB_CPP_UTIL_REF_COUNT_HPP_
#define BOLTDB_CPP_UTIL_REF_COUNT_HPP_

#include "boltdb/util/common.hpp"

namespace boltdb {

class RefCount {
 public:
  RefCount();
  RefCount(const RefCount& other);
  RefCount& operator=(const RefCount& other);

  ~RefCount();

  // Check if the object has a unique reference.
  bool unique() const { return count_ != nullptr && *count_ == 1; }

  // Get the number of references to the object.
  int count() const { return *count_; }

  // Releases the ownership of the managed object.
  void release();

 private:
  DISALLOW_MOVE(RefCount);

  void copy(const RefCount& other);

  mutable int* count_;
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_UTIL_REF_COUNT_HPP_