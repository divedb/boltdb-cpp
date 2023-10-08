#ifndef BOLTDB_CPP_STORAGE_NODE_HPP_
#define BOLTDB_CPP_STORAGE_NODE_HPP_

#include <cstdint>
#include <numeric>
#include <vector>

#include "boltdb/page/page.hpp"
#include "boltdb/util/common.hpp"
#include "boltdb/util/slice.hpp"
#include "boltdb/util/status.hpp"
#include "boltdb/util/types.hpp"

namespace boltdb {

class Bucket;

// Inode represents an internal node inside of a node.
// It can be used to point to elements in a page
// or point to an element which hasn't been added to a page yet.
// TODO(gc): any alternative to remove this structure?
struct Inode {
 public:
  u32 flags;
  PageID pgid;
  ByteSlice key;
  ByteSlice value;
};

// Node represents an in-memory, deserialized page.
class Node {
 public:
  static constexpr const int kMinLeafKeys = 1;
  static constexpr const int kMinBranchKeys = 2;

  Node(PageID pgid, Bucket* bucket, Node* parent)
      : pgid_(pgid), bucket_(bucket), parent_(parent) {}

  // Return the top-level node this node is attached to.
  Node* root() {
    if (parent_ == nullptr) {
      return this;
    }

    return parent_->root();
  }

  // Return true if this node is leaf, otherwise false.
  bool is_leaf() const { return is_leaf_; }
  std::size_t inode_count() const { return inodes_.size(); }

  // Get the minimum number of inodes this node should have.
  int min_keys() const {
    if (is_leaf_) {
      return kMinLeafKeys;
    }

    return kMinBranchKeys;
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

  // Initializes the node from a page.
  void read(Page* page);

  // Writes the items onto one or more pages.
  void write(Page* page);

  void append(Node* child) { children_.push_back(child); }

 private:
  // Find the first satisfied index such that inodes_[index].key >= key.
  int index_of(ByteSlice key);

  // Writes the nodes to dirty pages and splits nodes as it goes.
  // Return an error if the dirty pages cannot be allocated.
  Status spill();

  // Breaks up a node into multiple smaller nodes, if appropriate.
  // This should only be called from the `spill()` function.
  std::vector<Node*> split(int page_size);

  // Breaks up a node into two smaller nodes, if appropriate.
  // This should only be called from the `split()` function.
  std::pair<Node*, Node*> split_two(int page_size);

  // Finds the position where a page will fill a given threshold.
  // It returns the index as well as the size of the first page.
  // This is only be called from split().
  std::pair<int, int> split_index(int threshold);

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