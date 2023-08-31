#ifndef BOLTDB_CPP_UTIL_REF_COUNT_HPP_
#define BOLTDB_CPP_UTIL_REF_COUNT_HPP_

#include "boltdb/util/common.hpp"

namespace boltdb {

class RefCount {
 public:
  RefCount();
  RefCount(const RefCount& other);
  RefCount(RefCount&& other) noexcept;

  RefCount& operator=(const RefCount& other);
  // Note that: self movement makes trouble.
  // https://ericniebler.com/2017/03/31/post-conditions-on-self-move/
  RefCount& operator=(RefCount&& other) noexcept;

  ~RefCount();

  // Check if the object has a unique reference.
  bool unique() const { return count_ != nullptr && *count_ == 1; }

  // Get the number of references to the object.
  int count() const {
    if (count_ == nullptr) {
      return 0;
    }

    return *count_;
  }

  // Decrease the original counter by 1. If this is the last reference,
  // release the associated memory. Then, reset the state back to the initial
  // state, equivalent to the state after the default constructor is called.
  void reset();

 private:
  void decrease_and_try_release();
  void copy_from(const RefCount& other);
  void move_from(RefCount&& other);

  mutable int* count_{};
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_UTIL_REF_COUNT_HPP_