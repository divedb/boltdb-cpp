#include "boltdb/util/ref_count.hpp"

#include <assert.h>

namespace boltdb {

RefCount::RefCount() : count_(new int(1)) {}

RefCount::RefCount(const RefCount& other) { copy(other); }

RefCount& RefCount::operator=(const RefCount& other) {
  if (this == &other) {
    return *this;
  }

  release();
  copy(other);

  return *this;
}

RefCount::~RefCount() { release(); }

void RefCount::release() {
  if (count_ && --*count_ == 0) {
    delete count_;
    count_ = nullptr;
  }
}

void RefCount::reset() {
  release();
  count_ = new int(1); 
}

void RefCount::copy(const RefCount& other) {
  count_ = other.count_;
  *count_ = *count_ + 1;
}

}  // namespace boltdb