#ifndef BOLTDB_CPP_STORAGE_PAGE_HPP_
#define BOLTDB_CPP_STORAGE_PAGE_HPP_

#include <cstdint>
#include <string>

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

// Represents a generic page structure, which could be converted to `meta`,
// `freelist`, `branch` and `leaf` pages.
class Page {
 public:
  // Get page flag.
  [[nodiscard]] PageFlag flag() const;

  // Get number of leaf page elements.
  [[nodiscard]] u16 count() const;

  // Get number of overflow pages.
  [[nodiscard]] u32 overflow() const;

  // Get page id.
  [[nodiscard]] PageID pid() const;

  // Get a human readable page type string used for debugging.
  [[nodiscard]] std::string type() const;

  // Get a pointer to the metadata section of the page.
  [[nodiscard]] Meta* meta() const;

  // Get the leaf node by index.
  [[nodiscard]] LeafPageElement* leaf_page_element(u16 index) const;

  // TODO(gc): fix this
  [[nodiscard]] LeafPageElement* leaf_page_elements() const;

  // Get the branch node by index.
  [[nodiscard]] BranchPageElement* branch_page_element(u16 index) const;

  // TODO(gc): fix this
  [[nodiscard]] BranchPageElement* branch_page_elements() const;

  void hexdump(int n) const;

 private:
  // Cast base ptr of this page to `T` type.
  template <typename T>
  T* cast() const;

  PageFlag flag_;  // 2 bytes, identify page type
  u16 count_;      // 2 bytes
  u32 overflow_;   // 4 bytes, number of overflow pages
  PageID pid_;     // 8 bytes, page id
};

// BranchPageElement represents a node on a branch page.
class BranchPageElement {
 public:
  // Get a byte slice of the node key.
  [[nodiscard]] ByteSlice key() const;

 private:
  u32 key_size_;
  PageID pid_;
};

// LeafPageElement represents a node on a leaf page.
class LeafPageElement {
 public:
  // Get a byte slice of the node key.
  [[nodiscard]] ByteSlice key() const;

  // Get a byte slice of the node value.
  [[nodiscard]] ByteSlice value() const;

 private:
  u32 flags_;
  u32 key_size_;
  u32 value_size_;
};

static constexpr const int kPageHeaderSize = sizeof(Page);
static constexpr const int kBranchPageElementSize = sizeof(BranchPageElement);
static constexpr const int kLeafPageElementSize = sizeof(LeafPageElement);

}  // namespace boltdb

#endif