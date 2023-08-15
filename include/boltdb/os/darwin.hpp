#ifndef BOTLDB_CPP_OS_DARWIN_HPP_
#define BOTLDB_CPP_OS_DARWIN_HPP_

namespace boltdb {

class OS {
 public:
  // Get the number of bytes in a page. Page granularity is the granularity of
  // many of the memory management calls.
  // The page size is a system page size and may not be the same as the
  // underlying hardward page size.
  static int getpagesize();
};

}  // namespace boltdb

#endif  // BOTLDB_CPP_OS_DARWIN_HPP_
