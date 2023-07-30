#include "boltdb/storage/page.hpp"

#include "boltdb/util/util.hpp"

using namespace boltdb;

std::string Page::type() const {
  if (flags & kBranch != 0) {
    return "branch";
  } else if (flags & kLeaf != 0) {
    return "leaf";
  } else if (flags & kMeta) {
    return "meta";
  } else if (flags & kFreeList) {
    return "freelist";
  } else {
    return format("unknown<%02x>", flags);
  }
}

ByteSlice BranchPageElement::key() const { return ByteSlice(vptr, key_size); }

ByteSlice LeafPageElement::key() const { return ByteSlice(vptr, key_size); }

ByteSlice LeafPageElement::value() const {
  return ByteSlice(vptr + key_size, value_size);
}
