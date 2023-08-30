#include "boltdb/util/ref_count.hpp"

#include <iostream>

namespace boltdb {

RefCount::RefCount() { reset(); }

RefCount::RefCount(const RefCount& other) { copy_from(other); }

RefCount::RefCount(RefCount&& other) { move_from(std::move(other)); }

RefCount& RefCount::operator=(const RefCount& other) {
  if (this == &other) {
    return *this;
  }

  decrease_and_try_release();
  copy_from(other);

  return *this;
}

RefCount& RefCount::operator=(RefCount&& other) {
  decrease_and_try_release();
  move_from(std::move(other));

  return *this;
}

RefCount::~RefCount() { decrease_and_try_release(); }

void RefCount::decrease_and_try_release() {
  if (count_ && --*count_ == 0) {
    delete count_;
    count_ = nullptr;
  }
}

void RefCount::reset() {
  decrease_and_try_release();
  count_ = new int(1);
}

void RefCount::copy_from(const RefCount& other) {
  count_ = other.count_;
  *count_ = *count_ + 1;
}

void RefCount::move_from(RefCount&& other) {
  count_ = other.count_;
  other.count_ = nullptr;
}

}  // namespace boltdb