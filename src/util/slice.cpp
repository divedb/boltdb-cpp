#include "boltdb/util/slice.hpp"

#include <algorithm>
#include <cstring>

#include "boltdb/util/util.hpp"

using namespace boltdb;

ByteSlice::ByteSlice()
    : _data(nullptr), _size(0), _cap(0), _ref_count(nullptr) {}

ByteSlice::ByteSlice(Byte const* data, std::size_t size) {
  // TODO(gc): validate argument (NULL)
  auto& pool = MemoryPool::instance();

  // Add extra 1 to termainate.
  _size = std::min(strlen(data), size);
  _cap = round_up_to_power_of_two(_size + 1);
  _data = pool.allocate(_cap);
  _ref_count = reinterpret_cast<int*>(pool.allocate(sizeof(int)));

  *_ref_count = 1;
  std::strncpy(_data, data, _size);
}

ByteSlice::ByteSlice(Byte const* data) : ByteSlice(data, strlen(data)) {}

ByteSlice::ByteSlice(std::string const& data)
    : ByteSlice(data.c_str(), data.size()) {}

ByteSlice::~ByteSlice() { destroy(); }

ByteSlice::ByteSlice(ByteSlice& other) {
  shallow_copy(other);
  ++*(other._ref_count);
}

ByteSlice& ByteSlice::operator=(ByteSlice& other) {
  if (this == &other) {
    return *this;
  }

  destroy();
  shallow_copy(other);
  ++*(other._ref_count);

  return *this;
}

ByteSlice::ByteSlice(ByteSlice&& other) {
  shallow_copy(other);
  other.reset();
}

ByteSlice& ByteSlice::operator=(ByteSlice&& other) {
  if (this == &other) {
    return *this;
  }

  shallow_copy(other);
  other.reset();

  return *this;
}

void ByteSlice::destroy() {
  if (!_data || !_ref_count) {
    return;
  }

  int count = --*_ref_count;

  if (count == 0) {
    delete _ref_count;
    MemoryPool::instance().deallocate(_data, _cap);
  }
}

void ByteSlice::shallow_copy(ByteSlice& other) {
  _data = other._data;
  _size = other._size;
  _cap = other._cap;
  _ref_count = other._ref_count;
}

void ByteSlice::reset() {
  _data = nullptr;
  _size = 0;
  _cap = 0;
  _ref_count = nullptr;
}