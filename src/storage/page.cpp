#include "boltdb/storage/page.hpp"

#include "boltdb/util/util.hpp"

using namespace boltdb;

inline PageFlag Page::flag() const { return _flag; }

inline u16 Page::count() const { return _count; }

inline u32 Page::overflow() const { return _overflow; }

inline PageID Page::pid() const { return _pid; }

inline Meta* Page::meta() const { return cast<Meta*>(); }

inline LeafPageElement* Page::leaf_page_element(u16 index) const {
  LeafPageElement* base = cast<LeafPageElement>();

  return &base[index];
}

inline BranchPageElement* Page::branch_page_element(u16 index) const {
  BranchPageElement* base = cast<BranchPageElement>();

  return &base[index];
}

void Page::hexdump(int n) const {}

template <typename T>
T* Page::cast() const {
  return reinterpret_cast<T*>(reinterpret_cast<Byte*>(this) + kPageHeaderSize);
}

ByteSlice BranchPageElement::key() const { return ByteSlice(vptr, key_size); }

ByteSlice LeafPageElement::key() const { return ByteSlice(vptr, key_size); }

ByteSlice LeafPageElement::value() const { return ByteSlice(vptr + key_size, value_size); }

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
