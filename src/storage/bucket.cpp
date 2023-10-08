#include "boltdb/storage/bucket.hpp"

#include "boltdb/page/node.hpp"
#include "boltdb/transaction/txn.hpp"
#include "boltdb/util/exception.hpp"
#include "boltdb/util/util.hpp"

namespace boltdb {

Bucket::Bucket(Txn* txn) : txn_(txn) {
  if (txn->is_writable()) {
    // TODO
  }
}

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

std::unique_ptr<Cursor> Bucket::cursor() {
  // Update transaction statistics.
  txn_->stats.cursor_count++;

  // Allocate and return a cursor.
  return std::make_unique<Cursor>(this);
}

std::pair<Page*, Node*> Bucket::page_node(PageID pgid) {
  // Inline buckets have a fake page embedded in their value so treat them
  // differently. We'll return the rootNode (if available) or the fake page.
  if (bucket_meta_.root == 0) {
    if (id != 0) {
      std::string error =
          format("inline bucket non-zero page access(2): %d != 0", id);
      throw DBException(error);
    }

    if (root_node_ != nullptr) {
      return {nullptr, root_node_};
    }

    return {page_, nullptr};
  }

  // Check the node cache for non-inline buckets.
  if (auto iter = node_cache_.find(id); ite != node_cache_.end()) {
    return {iter->first, nullptr};
  }

  // Finally lookup the page from the transaction if no node is materialized.
  return {txn_->page(id), nullptr};
}

}  // namespace boltdb