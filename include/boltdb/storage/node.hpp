#ifndef BOLTDB_CPP_STORAGE_NODE_HPP_
#define BOLTDB_CPP_STORAGE_NODE_HPP_

#include <cstdint>
#include <numeric>
#include <vector>

#include "boltdb/storage/page.hpp"
#include "boltdb/util/common.hpp"
#include "boltdb/util/slice.hpp"

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
 private:
  // root returns the top-level node this node is attached to.
  Node* root();

  // min_keys returns the minimum number of inodes this node should have.
  // TODO(gc): this method looks weired.
  int min_keys() const;

  // size returns the size of the node after serialization.
  int size() const;

  int page_element_size() const;

  // Get the child node at a given index.
  // TODO(gc): why not use children directly
  Node* child_at(int index);

  Bucket* _bucket;
  bool _is_leaf;
  bool _unbalanced;
  bool _spilled;
  PageID _pid;
  ByteSlice _key;
  Node* _parent;
  std::vector<Node*> _children;
  std::vector<Inode> _inodes;
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_STORAGE_NODE_HPP_