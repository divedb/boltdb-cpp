#include "boltdb/db/cursor.hpp"

#include "boltdb/db/bucket.hpp"
#include "boltdb/page/node.hpp"
#include "boltdb/page/page.hpp"

namespace boltdb {

std::pair<ByteSlice, ByteSlice> Cursor::first() {
  stack_.clear();

  auto [p, n] = bucket_->page_node(bucket_->root());
  stack_.emplace_back(p, n, 0);
}

void Cursor::move_to_leaf() {
  while (true) {
    auto& ref = stack_.back();

    if (ref.is_leaf()) {
      break;
    }

    // Keep adding pages pointing to the first element to the stack.
    PageID pgid;

    if (ref.node != nullptr) {
      pgid = ref.
    }
  }
}

inline bool ElemRef::is_leaf() const {
  if (node != nullptr) {
    return node->is_leaf();
  }

  return page->flag() == PageFlag::kLeaf;
}

inline int ElemRef::count() const {
  if (node != nullptr) {
    return node->inode_count();
  }

  return page->count();
}

}  // namespace boltdb