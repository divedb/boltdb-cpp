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

class FreeList;

enum PageFlag : u16 { kInvalid = 0x00, kBranch = 0x01, kLeaf = 0x02, kMeta = 0x04, kFreeList = 0x08 };

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
  Page(PageID pgid, PageFlag flag, int page_size);

  // Accessor.
  PageFlag flag() const { return pheader_->flag; }
  u16 count() const { return pheader_->count; }
  u32 overflow() const { return pheader_->overflow; }
  PageID id() const { return pheader_->pgid; }

  // Modifier.
  void set_flag(PageFlag flag) { pheader_->flag = flag; }
  void set_count(u16 count) { pheader_->count = count; }
  void set_overflow(u32 overflow) { pheader_->overflow = overflow; }

  // Get a human readable page type string used for debugging.
  std::string type() const;

  // Get underlying page data.
  const Byte* data() const { return pdata_.data(); }
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

  // Get the raw page data without the page header.
  Byte* skip_page_header();
  const Byte* skip_page_header() const;

  // Hex dump the page content for debug.
  void hexdump(std::ostream& os) const;

 private:
  friend class FreeList;

  template <typename T>
  T* cast_ptr() const;

  int page_size_;        // Page size
  PageHeader* pheader_;  // Page header
  ByteSlice pdata_;      // Page data
};

// BranchPageElement represents a node on a branch page.
class BranchPageElement {
 public:
  BranchPageElement(u32 pos, u32 key_size, PageID pgid) : pos(pos), key_size(key_size), pgid(pgid) {}

  // Get a byte slice of the node key.
  ByteSlice key() const;

  u32 pos;
  u32 key_size;
  PageID pgid;
};

// LeafPageElement represents a node on a leaf page.
class LeafPageElement {
 public:
  LeafPageElement(u32 flags, u32 pos, u32 key_size, u32 value_size)
      : flags(flags), pos(pos), key_size(key_size), value_size(value_size) {}

  // Get a byte slice of the node key.
  ByteSlice key() const;

  // Get a byte slice of the node value.
  ByteSlice value() const;

  u32 flags;
  u32 pos;
  u32 key_size;
  u32 value_size;
};

static constexpr const int kMinKeysPerPage = 2;
static constexpr const int kPageHeaderSize = sizeof(PageHeader);
static constexpr const int kBranchPageElementSize = sizeof(BranchPageElement);
static constexpr const int kLeafPageElementSize = sizeof(LeafPageElement);

}  // namespace boltdb

#endif