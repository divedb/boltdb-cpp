#include "boltdb/storage/node.hpp"

#include <exception>

#include "boltdb/util/exception.hpp"
#include "boltdb/util/util.hpp"

using namespace boltdb;

inline Node* Node::root() {
  if (_parent == nullptr) {
    return this;
  }

  return _parent->root();
}

inline int Node::min_keys() const {
  if (_is_leaf) {
    return 1;
  }

  return 2;
}

// The size formula is based on:
// Page Header | Page Element Size1 + Len(key1) + Len(value1) ... Page Element
// SizeN + Len(keyN) + Len(valueN)
inline int Node::size() const {
  int elsz = page_element_size();

  return std::accumulate(_inodes.begin(), _inodes.end(), kPageSize,
                         [elsz](int init, const Inode& inode) {
                           return elsz + init + inode.key.size() +
                                  inode.value.size();
                         });
}

inline int Node::page_element_size() const {
  if (_is_leaf) {
    return kLeafPageElementSize;
  }

  return kBranchPageElementSize;
}

inline Node* Node::child_at(int index) {
  if (_is_leaf) {
    std::string err = format("invalid child at (%d) on a leaf node", index);
    throw DBException(err);
  }

  return bucket->node(_inodes[index].pgid, this);
}