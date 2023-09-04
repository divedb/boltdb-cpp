#include "boltdb/storage/node.hpp"

#include <exception>

#include "boltdb/storage/page.hpp"
#include "boltdb/util/exception.hpp"
#include "boltdb/util/util.hpp"

namespace boltdb {

// The size formula is based on:
// Page Header | Page Element Size1 + Len(key1) + Len(value1) ... Page Element
// SizeN + Len(keyN) + Len(valueN)
int Node::size() const {
  int elsz = page_element_size();

  return std::accumulate(inodes_.begin(), inodes_.end(), kPageHeaderSize,
                         [elsz](int init, const Inode& inode) {
                           return init + elsz + inode.key.size() +
                                  inode.value.size();
                         });
}

bool Node::is_size_less_than(int v) const {
  int sz = kPageHeaderSize;
  int elsz = page_element_size();

  for (auto&& inode : inodes_) {
    sz += elsz + inode.key.size() + inode.value.size();

    if (sz >= v) {
      return false;
    }
  }

  return true;
}

Node* Node::child_at(int index) const {
  if (_is_leaf) {
    std::string err = format("invalid child at (%d) on a leaf node", index);
    throw DBException(err);
  }

  return bucket->node(_inodes[index].pgid, this);
}

int Node::child_index(Node* child) const {
  auto iter = std::lower_bound(
      inodes_.begin(), inodes_.end(), child->key_,
      [](ByteSlice lhs, ByteSlice rhs) { return lhs.bytes_compare(rhs) >= 0; });

  return std::distance(inodes_.begin(), iter);
}

int Node::num_children() const { return inodes_.size(); }

Node* Node::next_sibling() const {
  if (parent_ == nullptr) {
    return nullptr;
  }

  int index = parent_->child_index(this);

  if (index >= parent_->num_children() - 1) {
    return nullptr;
  }

  return parent_->child_at(index + 1);
}

Node* Node::prev_sibling() const {
  if (parent_ == nullptr) {
    return nullptr;
  }

  int index = parent_->child_index(this);

  if (index == 0) {
    return nullptr;
  }

  return parent_->child_at(index - 1);
}

void Node::put(ByteSlice old_key, ByteSlice new_key, ByteSlice value,
               PageID pgid, u32 flags) {
  std::string error;
  PageID high_mark = bucket_->txn()->meta().pgid;

  if (pgid >= high_mark) {
    error = format("pgid (%d) above high water mark (%d)", pgid, high_mark);

    throw NodeException(error);
  }
}

}  // namespace boltdb