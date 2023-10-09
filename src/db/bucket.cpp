#include "boltdb/db/bucket.hpp"

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

bool Bucket::node(PageID pgid, Node* parent, Node*& out_node) {
  // Retrieve node if it's already been created.
  if (auto iter = node_cache_.find(pgid); iter != node_cache_.end()) {
    out_node = iter->second;

    return true;
  }

  // Otherwise create a node and cache ite.
  Node* n = new Node(pgid, this, parent);

  if (parent == nullptr) {
    root_node_ = n;
  }

  // Use the inline page if this is an inline bucket.
  Page* p = page_;

  if (p == nullptr) {
    p = txn_->page(pgid);
  }

  // Read the page into the node and cache it.
  n->read(p);
  node_cache_[pgid] = n;

  out_node = n;

  // Update statistics.
  txn_->stats.node_count++;

  return false;
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