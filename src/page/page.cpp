#include "boltdb/storage/page.hpp"

#include <iostream>
#include <sstream>
#include <type_traits>

#include "boltdb/util/binary.hpp"
#include "boltdb/util/util.hpp"

namespace boltdb {

template <typename Pointer>
Byte* advance_n_bytes(Pointer p, std::size_t n) {
  return std::next(reinterpret_cast<Byte*>(p), n);
}

template <typename T>
Byte* advance_n_bytes(const T* p, std::size_t n) {
  return advance_n_bytes(const_cast<T*>(p), n);
}

Page::Page(PageID pid, PageFlag flag, int page_size)
    : pheader_(pid, flag), page_size_(page_size) {
  pdata_.reserve(page_size_);
  binary::BigEndian::append_variadic_uint(pdata_, pheader_.pid,
                                          static_cast<u16>(pheader_.flag),
                                          pheader_.count, pheader_.overflow);
}

std::string Page::type() const {
  auto flag = pheader_.flag;

  if ((flag & kBranch) != 0) {
    return "branch";
  }

  if ((flag & kLeaf) != 0) {
    return "leaf";
  }

  if ((flag & kMeta) != 0) {
    return "meta";
  }

  if ((flag & kFreeList) != 0) {
    return "freelist";
  }

  return format("unknown<%02x>", flag);
}

Meta* Page::meta() const { return cast_ptr<Meta>(); }

LeafPageElement* Page::leaf_page_element(u16 index) const {
  auto base = cast_ptr<LeafPageElement>();

  return &base[index];
}

std::span<LeafPageElement> Page::leaf_page_elements() const {
  auto base = cast_ptr<LeafPageElement>();

  return {base, pheader_.count};
}

BranchPageElement* Page::branch_page_element(u16 index) const {
  auto base = cast_ptr<BranchPageElement>();

  return &base[index];
}

std::span<BranchPageElement> Page::branch_page_elements() const {
  auto base = cast_ptr<BranchPageElement>();

  return {base, pheader_.count};
}

void Page::hexdump(int n) const {
  std::ostringstream oss;
  Byte* base = skip_page_header();

  // TODO(gc): avoid large n
  for (int i = 0; i < n; i++) {
    oss << format("02x", *base);
    std::advance(base, 1);
  }

  std::cout << oss.str();
}

// Get base ptr, which excludes the page header.
inline Byte* Page::skip_page_header() const {
  return advance_n_bytes(data(), kPageHeaderSize);
}

template <typename T>
inline T* Page::cast_ptr() const {
  Byte* base = skip_page_header();

  return reinterpret_cast<T*>(base);
}

ByteSlice BranchPageElement::key() const {
  Byte* key = advance_n_bytes(this, pos_);

  return {key, key + key_size_};
}

ByteSlice LeafPageElement::key() const {
  Byte* key = advance_n_bytes(this, pos_);

  return {key, key + key_size_};
}

ByteSlice LeafPageElement::value() const {
  Byte* value = advance_n_bytes(this, pos_ + key_size_);

  return {value, value + value_size_};
}

}  // namespace boltdb