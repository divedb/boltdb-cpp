#ifndef BOLTDB_CPP_STORAGE_PAGE_HPP_
#define BOLTDB_CPP_STORAGE_PAGE_HPP_

#include <cstdint>

#include "boltdb/util/common.hpp"
#include "boltdb/util/slice.hpp"

namespace boltdb {

enum PageFlag : u16 {
  kBranch = 0x01,
  kLeaf = 0x02,
  kMeta = 0x04,
  kFreeList = 0x08
};

enum LeafFlag : u16 { kBucket = 0x01 };

struct Page {
 public:
  // type returns a human reabable page type string used for debugging.
  std::string type() const;

  PageID pid;
  PageFlag flags;
  u16 count;
  u32 overflow;  // Next page id?
  char ptr[];
};

// BranchPageElement represents a node on a branch page.
// TODO(gc): is it redundant to keep the pgid.
struct BranchPageElement {
 public:
  ByteSlice key() const;

  u32 key_size;
  PageID pgid;
  Byte vptr[];
};

// LeafPageElement represents a node on a leaf page.
struct LeafPageElement {
 public:
  // key returns a byte slice of the node key.
  ByteSlice key() const;

  // value returns a byte slice of the node value.
  ByteSlice value() const;

  u32 flags;
  u32 key_size;
  u32 value_size;
  Byte vptr[];
};

static constexpr const int kPageSize = sizeof(Page);
static constexpr const int kBranchPageElementSize = sizeof(BranchPageElement);
static constexpr const int kLeafPageElementSize = sizeof(LeafPageElement);

}  // namespace boltdb

#endif