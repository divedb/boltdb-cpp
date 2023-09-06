#ifndef BOLTDB_CPP_STORAGE_PAGE_HPP_
#define BOLTDB_CPP_STORAGE_PAGE_HPP_

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "boltdb/util/slice.hpp"
#include "boltdb/util/types.hpp"

namespace boltdb {

enum PageFlag : u16 {
  kBranch = 0x01,
  kLeaf = 0x02,
  kMeta = 0x04,
  kFreeList = 0x08
};

enum LeafFlag : u16 { kBucket = 0x01 };

// Forward declaration.
class Meta;
class LeafPageElement;
class BranchPageElement;

class PageHeader {
 public:
  PageHeader(PageID pgid, PageFlag flag) : pgid(pgid), flag(flag) {}

  PageID pgid;     // 8 bytes, page id
  PageFlag flag;   // 2 bytes, identify page type
  u16 count{};     // 2 bytes
  u32 overflow{};  // 4 bytes, number of overflow pages
};

// `Page` represents a generic page structure, which could be converted to
// `meta`, `freelist`, `branch` and `leaf` pages.
class Page {
 public:
  Page(PageID pid, PageFlag flag, int page_size);

  // Get and set page flag.
  PageFlag flag() const { return pheader_.flag; }
  void set_flag(PageFlag flag) { pheader_.flag = flag; }

  // Get and set number of leaf page elements.
  u16 count() const { return pheader_.count; }
  void set_count(u16 count) { pheader_.count = count; }

  // Get number of overflow pages.
  u32 overflow() const { return pheader_.overflow; }

  // Get page id.
  PageID id() const { return pheader_.pgid; }

  // Get a human readable page type string used for debugging.
  std::string type() const;

  // Get underlying page data.
  Byte* data() { return pdata_.data(); }

  // Get page size in bytes.
  std::size_t page_size() const { return page_size_; }

  // Get a pointer to the metadata section of the page.
  Meta* meta() const;

  // Get the leaf node by index.
  LeafPageElement* leaf_page_element(u16 index) const;

  std::span<LeafPageElement> leaf_page_elements() const;

  // Get the branch node by index.
  BranchPageElement* branch_page_element(u16 index) const;

  std::span<BranchPageElement> branch_page_elements() const;

  void hexdump(int n) const;

 private:
  Byte* skip_page_header() const;

  template <typename T>
  T* cast_ptr() const;

  PageHeader pheader_;   // Page header
  ByteSlice pdata_;      // Page data
  const int page_size_;  // Page size
};

// BranchPageElement represents a node on a branch page.
class BranchPageElement {
 public:
  BranchPageElement(u32 pos, u32 key_size, PageID pid)
      : pos_(pos), key_size_(key_size), pid_(pid) {}

  u32 pos() const { return pos_; }
  u32 key_size() const { return key_size_; }
  PageID id() const { return pid_; }

  // Get a byte slice of the node key.
  ByteSlice key() const;

 private:
  u32 pos_;
  u32 key_size_;
  PageID pid_;
};

// LeafPageElement represents a node on a leaf page.
class LeafPageElement {
 public:
  LeafPageElement(u32 flags, u32 pos, u32 key_size, u32 value_size)
      : flags_(flags),
        pos_(pos),
        key_size_(key_size),
        value_size_(value_size) {}

  u32 flags() const { return flags_; }
  u32 pos() const { return pos_; }
  u32 key_size() const { return key_size_; }
  u32 value_size() const { return value_size_; }

  // Get a byte slice of the node key.
  ByteSlice key() const;

  // Get a byte slice of the node value.
  ByteSlice value() const;

 private:
  u32 flags_;
  u32 pos_;
  u32 key_size_;
  u32 value_size_;
};

static constexpr const int kPageHeaderSize = sizeof(PageHeader);
static constexpr const int kBranchPageElementSize = sizeof(BranchPageElement);
static constexpr const int kLeafPageElementSize = sizeof(LeafPageElement);

}  // namespace boltdb

#endif