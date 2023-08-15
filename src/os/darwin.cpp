#include "boltdb/os/darwin.hpp"

#include <unistd.h>

namespace boltdb {

int OS::getpagesize() { return ::getpagesize(); }

}  // namespace boltdb