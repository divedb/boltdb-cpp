#include "boltdb/page/page.hpp"

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

Page::Page(PageID pgid, PageFlag flag, int page_size) : page_size_(page_size) {
  pdata_.reserve(page_size_);
  pdata_ = binary::LittleEndian::append_variadic_uint(
      pdata_, pgid, static_cast<u16>(flag), static_cast<u16>(0),
      static_cast<u32>(0));
  pheader_ = reinterpret_cast<PageHeader*>(pdata_.data());
}

std::string Page::type() const {
  auto flag = pheader_->flag;

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

  return {base, pheader_->count};
}

BranchPageElement* Page::branch_page_element(u16 index) const {
  auto base = cast_ptr<BranchPageElement>();

  return &base[index];
}

std::span<BranchPageElement> Page::branch_page_elements() const {
  auto base = cast_ptr<BranchPageElement>();

  return {base, pheader_->count};
}

void Page::hexdump(std::ostream& os) const {
  std::ostringstream oss;
  const Byte* base = data();

  int i;
  int step = 0x10;
  int n = (page_size_ / step) * step;
  int r = page_size_ % step;

  for (i = 0; i < n; i += step) {
    oss << format("%04x - %04x    ", i, i + step - 1);

    for (int j = 0; j < step; j++) {
      oss << format("%02x ", *base++);
    }

    oss << '\n';
  }

  for (int j = 0; j < r; j++) {
    oss << format("02x", *base++);
  }

  if (r != 0) {
    oss << '\n';
  }

  os << oss.str();
}

// Get base ptr, which excludes the page header.
Byte* Page::skip_page_header() {
  return advance_n_bytes(data(), kPageHeaderSize);
}

const Byte* Page::skip_page_header() const {
  return advance_n_bytes(data(), kPageHeaderSize);
}

// TODO(gc): any better representation?
template <typename T>
T* Page::cast_ptr() const {
  Byte* base = const_cast<Byte*>(skip_page_header());

  return reinterpret_cast<T*>(base);
}

ByteSlice BranchPageElement::key() const {
  Byte* key = advance_n_bytes(this, pos);

  return {key, key + key_size};
}

ByteSlice LeafPageElement::key() const {
  Byte* key = advance_n_bytes(this, pos);

  return {key, key + key_size};
}

ByteSlice LeafPageElement::value() const {
  Byte* value = advance_n_bytes(this, pos + key_size);

  return {value, value + value_size};
}

}  // namespace boltdb