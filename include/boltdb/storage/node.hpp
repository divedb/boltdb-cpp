#ifndef BOLTDB_CPP_STORAGE_NODE_HPP_
#define BOLTDB_CPP_STORAGE_NODE_HPP_

#include <cstdint>
#include <numeric>
#include <vector>

#include "boltdb/storage/page.hpp"
#include "boltdb/util/common.hpp"
#include "boltdb/util/slice.hpp"
#include "boltdb/util/types.hpp"

namespace boltdb {

class Bucket;

// Inode represents an internal node inside of a node.
// It can be used to point to elements in a page or point to an element which
// hasn't been added to a paget net.
// TODO(gc): any alternative to remove this structure?
struct Inode {
  uint32_t flags;
  PageID pgid;
  ByteSlice key;
  ByteSlice value;
};

// Node represents an in-memory, deserialized page.
// TODO(gc):
// Q1: why node binds to a bucket. A bucket is a B+ tree?
class Node {
 public:
  Node(PageID pgid, Bucket* bucket, Node* parent)
      : pgid_(pgid), bucket_(bucket), parent_(parent) {}

  // Get the top-level node this node is attached to.
  Node* root() {
    if (parent_ == nullptr) {
      return this;
    }

    return parent_->root();
  }

  // Get the minimum number of inodes this node should have.
  int min_keys() const {
    if (is_leaf_) {
      return 1;
    }

    return 2;
  }

  // Get the size of the node after serialization.
  int size() const;

  // Get true if the node is less than a given size.
  // This is an optimization to avoid calculating a large node when we only need
  // to know if it fits inside a certain page size.
  bool is_size_less_than(int v) const;

  int page_element_size() const {
    if (is_leaf_) {
      return kLeafPageElementSize;
    }

    return kBranchPageElementSize;
  }

  // Get the child node at a given index.
  // TODO(gc): why not use children directly
  Node* child_at(int index);

  // Get the index of a given child node.
  int child_index(const Node* child) const;

  // Get the number of children.
  int num_children() const;

  // Get the next node with the same parent.
  Node* next_sibling() const;

  // Get previous node with the same parent.
  Node* prev_sibling() const;

  // Insert a key/value.
  // TODO(gc): why needs pgid and old_key parameters.
  void put(ByteSlice old_key, ByteSlice new_key, ByteSlice value, PageID pgid,
           u32 flags);

  // Remove a key from the node.
  void remove(ByteSlice key);

  void append(Node* child) { children_.push_back(child); }

 private:
  // Find the first satisfied index such that inodes_[index].key >= key.
  int index_of(ByteSlice key);

  bool is_leaf_{};
  bool unbalanced_{};
  bool spilled_{};
  PageID pgid_;
  Bucket* bucket_;
  Node* parent_;
  ByteSlice key_;
  std::vector<Node*> children_;
  std::vector<Inode> inodes_;
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_STORAGE_NODE_HPP_