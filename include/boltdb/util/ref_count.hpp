#ifndef BOLTDB_CPP_UTIL_REF_COUNT_HPP_
#define BOLTDB_CPP_UTIL_REF_COUNT_HPP_

namespace boltdb {

class RefCount {
 public:
  RefCount();
  RefCount(RefCount& other);
  RefCount& operator=(RefCount& other);
  RefCount(RefCount&& other);
  RefCount&& operator=(RefCount&& other);

  ~RefCount();

  // Get the reference count.
  // Note that: 0 reference count is allowed when this only object is moved.
  int ref_count() const {
    if (!_count) {
      return 0;
    }

    return *_count;
  }

 private:
  void copy(RefCount& other);
  void move(RefCount& other);

  int* _count;
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_UTIL_REF_COUNT_HPP_