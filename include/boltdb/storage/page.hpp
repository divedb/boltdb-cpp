#ifndef BOLTDB_CPP_STORAGE_PAGE_HPP_
#define BOLTDB_CPP_STORAGE_PAGE_HPP_

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

// `Page` represents a generic page structure, which could be converted to
// `meta`, `freelist`, `branch` and `leaf` pages.
class Page {
 public:
  Page(PageID pid, PageFlag flag, int page_size);

  // Get page flag.
  [[nodiscard]] PageFlag flag() const { return flag_; }

  // Get number of leaf page elements.
  [[nodiscard]] u16 count() const { return count_; }

  // Get number of overflow pages.
  [[nodiscard]] u32 overflow() const { return overflow_; }

  // Get page id.
  [[nodiscard]] PageID pid() const { return pid_; }

  // Get a human readable page type string used for debugging.
  [[nodiscard]] std::string type() const;

  // Get underlying page data.
  const Byte* data() const { return pdata_.data(); }

  // Get page size in bytes.
  std::size_t page_size() const { return pdata_.size(); }

  // Get a pointer to the metadata section of the page.
  [[nodiscard]] Meta* meta() const;

  // Get the leaf node by index.
  [[nodiscard]] LeafPageElement* leaf_page_element(u16 index) const;

  [[nodiscard]] std::span<LeafPageElement> leaf_page_elements() const;

  // Get the branch node by index.
  [[nodiscard]] BranchPageElement* branch_page_element(u16 index) const;

  [[nodiscard]] std::span<BranchPageElement> branch_page_elements() const;

  void hexdump(int n) const;

 private:
  Byte* skip_page_header() const;

  template <typename T>
  T* cast_ptr() const;

  PageFlag flag_;    // 2 bytes, identify page type
  u16 count_{};      // 2 bytes
  u32 overflow_{};   // 4 bytes, number of overflow pages
  PageID pid_;       // 8 bytes, page id
  ByteSlice pdata_;  // Page data
};

// BranchPageElement represents a node on a branch page.
class BranchPageElement {
 public:
  BranchPageElement(u32 pos, u32 key_size, PageID pid)
      : pos_(pos), key_size_(key_size), pid_(pid) {}

  // Get a byte slice of the node key.
  [[nodiscard]] ByteSlice key() const;

  u32 pos() const { return pos_; }
  u32 key_size() const { return key_size_; }
  PageID pid() const { return pid_; }

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
  [[nodiscard]] ByteSlice key() const;

  // Get a byte slice of the node value.
  [[nodiscard]] ByteSlice value() const;

 private:
  u32 flags_;
  u32 pos_;
  u32 key_size_;
  u32 value_size_;
};

static constexpr const int kPageHeaderSize = sizeof(Page);
static constexpr const int kBranchPageElementSize = sizeof(BranchPageElement);
static constexpr const int kLeafPageElementSize = sizeof(LeafPageElement);

}  // namespace boltdb

#endif