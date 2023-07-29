#include "boltdb/util/ref_count.hpp"

using namespace boltdb;

RefCount::RefCount() : _count(new int(1)) {}

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
  if (_count && --*_count == 0) {
    delete _count;
    _count = nullptr;
  }
}

void RefCount::copy(const RefCount& other) {
  _count = other._count;
  *_count = *_count + 1;
}
