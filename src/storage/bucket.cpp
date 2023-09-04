#include "boltdb/storage/bucket.hpp"

#include "boltdb/storage/node.hpp"

namespace boltdb {

Bucket::Bucket(Txn* txn) : txn_(txn) {}

Node* Bucket::node(PageID pgid, Node* parent) {
  // Retrieve node if it's already been created.
  if (node_cache_.find(pgid) != node_cache_.end()) {
    return node_cache_[pgid];
  }

  Node* n = new Node(pgid, this, parent);

  if (parent == nullptr) {
    root_node_ = n;
  } else {
    parent->append(n);
  }
}

}  // namespace boltdb