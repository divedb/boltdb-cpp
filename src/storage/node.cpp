#include "boltdb/storage/node.hpp"

#include <exception>

#include "boltdb/storage/bucket.hpp"
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

Node* Node::child_at(int index) {
  if (is_leaf_) {
    std::string err = format("invalid child at (%d) on a leaf node", index);
    throw DBException(err);
  }

  return bucket_->node(inodes_[index].pgid, this);
}

int Node::child_index(const Node* child) const {
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

  if (old_key.size() == 0) {
    error = format("put: zero-length old key");

    throw NodeException(error);
  }

  if (new_key.size() == 0) {
    error = format("put: zero-length new key");

    throw NodeException(error);
  }

  // Find insertion index.
  int index = index_of(old_key);

  // Add capacity and shift nodes if we don't have an exact match and need to
  // insert.
  auto exact = (!inodes_.empty() && index < inodes_.size() &&
                inodes_[index].key == old_key);
  if (!exact) {
    auto iter = std::next(inodes_.begin(), index);
    inodes_.insert(iter, Inode{});
  }

  inodes_[index].flags = flags;
  inodes_[index].key = new_key;
  inodes_[index].value = value;
  inodes_[index].pgid = pgid;
}

void Node::remove(ByteSlice key) {
  int index = index_of(key);

  // Exit if the key isn't found.
  if (index >= inodes_.size() || key != inodes_[index].key) {
    return;
  }

  // Delete inode from the node.
  auto iter = std::next(inodes_.begin(), index);
  inodes_.erase(iter);

  // Mark the node as needing rebalancing.
  // TODO(gc): why
  unbalanced_ = true;
}

void Node::read(Page* page) {
  pgid_ = page->id();
  is_leaf_ = (page->flag() & PageFlag::kLeaf) != 0;
  inodes_.reserve(page->count());

  if (is_leaf_) {
    for (int i = 0; i < page->count(); i++) {
      auto element = page->leaf_page_element(i);
      inodes_.push_back(Inode{.flags = element->flags(),
                              .key = element->key(),
                              .value = element->value()});
    }
  } else {
    for (int i = 0; i < page->count(); i++) {
      auto element = page->branch_page_element(i);
      inodes_.push_back(Inode{.pgid = element->id(), .key = element->key()});
    }
  }

  // Save first key so we can find the node in the parent when we spill.
  if (inodes_.size() > 0) {
    key_ = inodes_[0].key;

    assert(key_.size() > 0);
  }
}

void Node::write(Page* page) {
  // Initialize page.
  if (is_leaf_) {
    page->set_flag(PageFlag::kLeaf);
  } else {
    page->set_flag(PageFlag::kBranch);
  }

  // TODO(gc): why limit this?
  auto size = inodes_.size();

  if (size >= 0xFFFF) {
    std::string error =
        format("inode overflow: %d (pgid=%d)", size, page->id());
    throw NodeException(error);
  }

  page->set_count(inodes_.size());

  // Stop here if there are no items to write.
  if (page->count() == 0) {
    return;
  }
}

int Node::index_of(ByteSlice key) {
  auto iter = std::lower_bound(inodes_.begin(), inodes_.end(), key);

  int index = std::distance(inodes_.begin(), iter);
}

}  // namespace boltdb