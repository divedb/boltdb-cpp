#include "boltdb/util/slice.hpp"

#include <algorithm>
#include <cstring>
#include <iomanip>
#include <sstream>

#include "boltdb/util/util.hpp"

using namespace boltdb;

ByteSlice::ByteSlice() : _data(nullptr), _size(0), _cap(0) {}

ByteSlice::ByteSlice(const Byte* data, std::size_t size) {
  // TODO(gc): validate argument (NULL)
  auto& pool = MemoryPool::instance();

  // Add extra 1 to termainate.
  _size = std::min(strlen(data), size);
  _cap = round_up_to_power_of_two(_size + 1);
  _data = pool.allocate(_cap);

  std::strncpy(_data, data, _size);
}

ByteSlice::ByteSlice(const Byte* data) : ByteSlice(data, strlen(data)) {}

ByteSlice::ByteSlice(const std::string& data) : ByteSlice(data.c_str(), data.size()) {}

ByteSlice::ByteSlice(const ByteSlice& other)
    : _data(other._data), _size(other._size), _cap(other._cap), _ref_count(other._ref_count) {}

ByteSlice::~ByteSlice() { destroy(); }

ByteSlice& ByteSlice::operator=(const ByteSlice& other) {
  if (this == &other) {
    return *this;
  }

  destroy();
  copy(other);

  return *this;
}

inline std::string ByteSlice::to_string() const { return std::string(_data, _size); }

std::string ByteSlice::to_hex() const {
  if (size() == 0) {
    return "[]";
  }

  std::ostringstream oss;
  oss << '[';
  oss << std::hex << _data[0];

  for (int i = 1; i < _size; i++) {
    oss << ',' << std::hex << _data[i];
  }

  oss << ']';

  return oss.str();
}

void ByteSlice::destroy() {
  if (_ref_count.unique() && _data) {
    MemoryPool::instance().deallocate(_data, _cap);
  }
}

void ByteSlice::copy(const ByteSlice& other) {
  _data = other._data;
  _size = other._size;
  _cap = other._cap;
  _ref_count = other._ref_count;
}
