#include "boltdb/storage/page.hpp"

#include <iostream>
#include <sstream>
#include <type_traits>

#include "boltdb/util/util.hpp"

namespace boltdb {

template <typename Pointer>
Byte* advance_n_bytes(Pointer p, std::size_t n) {
  return std::next(reinterpret_cast<Byte*>(p), n);
}

template <typename Pointer>
Byte* advance_n_bytes(const Pointer* p, std::size_t n) {
  return advance_n_bytes(const_cast<Pointer*>(p), n);
}

inline Byte* Page::skip_page_header() const {
  return advance_n_bytes(pdata_.data(), kPageHeaderSize);
}

template <typename T>
inline T* Page::cast_ptr() const {
  // Get base ptr, which excludes the page header.
  Byte* base = skip_page_header();

  return reinterpret_cast<T*>(base);
}

Meta* Page::meta() const { return cast_ptr<Meta>(); }

LeafPageElement* Page::leaf_page_element(u16 index) const {
  auto base = cast_ptr<LeafPageElement>();

  return &base[index];
}

std::span<LeafPageElement> Page::leaf_page_elements() const {
  auto base = cast_ptr<LeafPageElement>();

  return {base, count_};
}

BranchPageElement* Page::branch_page_element(u16 index) const {
  auto base = cast_ptr<BranchPageElement>();

  return &base[index];

  std::span<BranchPageElement> Page::branch_page_elements() const {
    auto base = cast_ptr<BranchPageElement>();

    return {base, count_};
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

  std::string Page::type() const {
    if ((flag_ & kBranch) != 0) {
      return "branch";
    }

    if ((flag_ & kLeaf) != 0) {
      return "leaf";
    }

    if ((flag_ & kMeta) != 0) {
      return "meta";
    }

    if ((flag_ & kFreeList) != 0) {
      return "freelist";
    }

    return format("unknown<%02x>", flag_);
  }

  ByteSlice BranchPageElement::key() const {
    Byte* key = advance_n_bytes(this, pos_);

    return {key, key_size_};
  }

  ByteSlice LeafPageElement::key() const {
    Byte* key = advance_n_bytes(this, pos_);

    return {key, key_size_};
  }

  ByteSlice LeafPageElement::value() const {
    Byte* value = advance_n_bytes(this, pos_ + key_size_);

    return {value, value_size_};
  }

}  // namespace boltdb