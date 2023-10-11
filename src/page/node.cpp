#include "boltdb/page/node.hpp"

#include <exception>
#include <utility>

#include "boltdb/db/bucket.hpp"
#include "boltdb/db/db.hpp"
#include "boltdb/page/page.hpp"
#include "boltdb/transaction/txn.hpp"
#include "boltdb/util/exception.hpp"
#include "boltdb/util/util.hpp"

namespace boltdb {

// The byte size is computed as:
// Page Header | Page Element Size1 + Len(key1) + Len(value1) ...
//               Page Element SizeN + Len(keyN) + Len(valueN)
int Node::byte_size() const {
  int elsz = page_element_size();

  return std::accumulate(inodes_.begin(), inodes_.end(), kPageHeaderSize, [elsz](int init, const Inode& inode) {
    return init + elsz + inode.key.size() + inode.value.size();
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
    std::string err = format("invalid child_at(%d) on a leaf node", index);
    throw DBException(err);
  }

  Node* n = nullptr;
  bool cached = bucket_->node(inodes_[index].pgid, this, n);

  if (!cached) {
    children_.push_back(n);
  }
}

int Node::child_index(const Node* child) const {
  auto first = inodes_.begin();
  auto last = inodes_.end();
  auto iter = std::lower_bound(first, last, child->first_key_, std::less<ByteSlice>{});

  return std::distance(first, iter);
}

// TODO(gc): will the length of inodes_ and children_ diff?
int Node::num_children() const { return inodes_.size(); }

Node* Node::next_sibling() const {
  if (parent_ == nullptr) {
    return nullptr;
  }

  // Compute next sibling index.
  int index = parent_->child_index(this) + 1;

  if (index >= parent_->num_children()) {
    return nullptr;
  }

  return parent_->child_at(index);
}

Node* Node::prev_sibling() const {
  if (parent_ == nullptr) {
    return nullptr;
  }

  // Compute previous sibling index.
  int index = parent_->child_index(this) - 1;

  if (index < 0) {
    return nullptr;
  }

  return parent_->child_at(index);
}

void Node::put(ByteSlice old_key, ByteSlice new_key, ByteSlice value, PageID pgid, u32 flags) {
  std::string error;
  PageID high_mark = bucket_->txn()->meta_page_id();

  if (pgid >= high_mark) {
    error = format("pgid (%d) above high water mark (%d)", pgid, high_mark);

    throw NodeException(error);
  }

  if (old_key.is_empty()) {
    error = format("put: zero-length old key");

    throw NodeException(error);
  }

  if (new_key.is_empty()) {
    error = format("put: zero-length new key");

    throw NodeException(error);
  }

  // Find insertion index.
  int index = index_of(old_key);

  // Add capacity and shift nodes if we don't have an exact match and need to insert.
  auto exist = (!inodes_.empty() && index < inodes_.size() && inodes_[index].key == old_key);
  if (!exist) {
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

// TODO(gc): value and pgid can't exist at same time, try use variant.
// page no child page information.
void Node::read(const Page& page) {
  u16 count = page.count();

  pgid_ = page.id();
  is_leaf_ = (page.flag() & PageFlag::kLeaf) != 0;

  // TODO(gc): do we need to clear first?
  inodes_.reserve(page.count());

  if (is_leaf_) {
    for (auto i = 0; i < count; i++) {
      auto element = page.leaf_page_element(i);
      inodes_.emplace_back(element->flags, static_cast<PageID>(0), element->key(), element->value());
    }
  } else {
    for (auto i = 0; i < count; i++) {
      auto element = page.branch_page_element(i);
      inodes_.emplace_back(static_cast<u32>(PageFlag::kInvalid), element->pgid, element->key(), ByteSlice{});
    }
  }

  // Save first key so we can find the node in the parent when we spill.
  if (inodes_.size() > 0) {
    first_key_ = inodes_[0].key;

    assert(first_key_.size() > 0);
  }
}

// TODO(gc): figure out this
// If the length of key+value is larger than the max allocation size
// then we need to reallocate the byte array pointer.
//
// See: https://github.com/boltdb/bolt/pull/335
void Node::write(Page& page) {
  // Initialize page.
  if (is_leaf_) {
    page.set_flag(PageFlag::kLeaf);
  } else {
    page.set_flag(PageFlag::kBranch);
  }

  // TODO(gc): why limit this?
  // Both `LeafPageElement` and `BranchPageElement` header requires at least 32
  // bytes.
  // Suppose the page size is 16K, 16384/32=512. The maximum possible number of
  // elements is only 512.
  auto size = inodes_.size();

  if (size >= DB::kSpecialCount) {
    std::string error = format("inode overflow: %d (pgid=%d)", size, page->id());

    throw NodeException(error);
  }

  page.set_count(size);

  // Stop here if there are no items to write.
  if (page.count() == 0) {
    return;
  }

  // Loop over each item and write it to the page.
  // 1. Skip page header.
  // 2. Skip page element header.
  Byte* base = page.skip_page_header();
  base = std::next(base, inodes_.size() * page_element_size());

  // TODO(gc): add enumeration support.
  if (is_leaf_) {
    for (int i = 0; i < size; i++) {
      auto& inode = inodes_[i];
      auto& key = inode.key;
      auto& val = inode.value;

      auto element = page.leaf_page_element(static_cast<u16>(i));
      element->pos = std::distance(reinterpret_cast<Byte*>(element), base);
      element->flags = inode.flags;
      element->key_size = key.size();
      element->value_size = val.size();
      base = std::copy(key.data(), std::next(key.data(), key.size()), base);
    }
  } else {
    for (int i = 0; i < size; i++) {
      auto& inode = inodes_[i];
      auto& key = inode.key;
      auto& val = inode.value;
      auto element = page.branch_page_element(static_cast<u16>(i));

      element->pos = std::distance(reinterpret_cast<Byte*>(element), base);
      element->key_size = inode.key.size();
      element->pgid = inode.pgid;

      base = std::copy(key.data(), std::next(key.data(), key.size()), base);
    }
  }

  // DEBUG only: n.dump()
}

int Node::index_of(ByteSlice key) {
  auto iter = std::lower_bound(inodes_.begin(), inodes_.end(), key);

  int index = std::distance(inodes_.begin(), iter);
}

std::vector<Node*> Node::split(int page_size) {
  std::vector<Node*> nodes;
  Node* split_node = this;

  while (true) {
    auto [left, right] = split_node->split_two(page_size);
    nodes.push_back(left);

    // If we can't split then exit the loop.
    if (right == nullptr) {
      break;
    }

    // Continue to split right node.
    split_node = right;
  }

  return nodes;
}

// TODO(gc): take care of memory leak, should the node freed in the bucket or in this destructor?
std::pair<Node*, Node*> Node::split_two(int page_size) {
  // Ignore the split if the page doesn't have at least enough nodes for two
  // pages or if the nodes can fit in a single page.
  if (inodes_.size() <= 2 * kMinKeysPerPage || is_size_less_than(page_size)) {
    return std::make_pair(this, nullptr);
  }

  // Determine the threshold before starting a new node.
  // Fill percent must be in the range [kMinFillPercent, kMaxFillPercent].
  f64 fill_percent = bucket_->fill_percent();
  fill_percent = std::max(fill_percent, Bucket::kMinFillPercent);
  fill_percent = std::min(fill_percent, Bucket::kMaxFillPercent);

  int threshold = static_cast<int>(page_size * fill_percent);

  // Determine split position and sizes of the two pages.
  auto [index, _] = split_index(threshold);

  // Split node into two separate nodes.
  // If there's no parent then we'll need to create one.
  if (parent_ == nullptr) {
    parent_ = new Node(PageID{}, bucket_, nullptr);
    parent_->children_.push_back(this);
  }

  // Create a new node and add it to the parent.
  Node* next = new Node(PageID{}, bucket_, parent_);
  next->is_leaf_ = is_leaf_;
  parent_->children_.push_back(next);

  // Split inodes across two nodes.
  auto iter = std::next(inodes_.begin(), index);
  int size = std::distance(iter, inodes_.end());
  next->inodes_.resize(size);

  std::copy(iter, inodes_.end(), next->inodes_.begin());
  inodes_.resize(index);

  // Update the statistics.
  bucket_->txn()->stats.split++;

  return std::make_pair(this, next);
}

std::pair<int, int> Node::split_index(int threshold) {
  int i;
  int sz = kPageHeaderSize;

  // Loop until we only have the minimum number of keys
  // required for the second page.
  for (i = 0; i < inodes_.size() - kMinKeysPerPage; i++) {
    auto& inode = inodes_[i];
    int elsize = page_element_size() + inode.key.size() + inode.value.size();

    // If we have at least the minimum number of keys and adding another node
    // would put us over the threshold then exit and return.
    if (i >= kMinKeysPerPage && sz + elsize > threshold) {
      break;
    }

    // Add the element size to the total size.
    sz += elsize;
  }

  return std::make_pair(i, sz);
}

Status Node::spill() {
  auto txn = bucket_->txn();

  if (spilled_) {
    return {};
  }

  // Spill child nodes first. Child nodes can materialize sibling nodes in the
  // case of split-merge so we cannot use a range loop. We have to check the
  // children size on every iteration.
  std::sort(children_.begin(), children_.end(),
            [](const Node* lhs, const Node* rhs) { return lhs->inodes_[0].key < rhs->inodes_[0].key; });

  for (auto&& child : children_) {
    if (Status status = child->spill(); !status.ok()) {
      return status;
    }
  }

  // We no longer need the child list because it's only used for spill tracking.
  // TODO(gc): memory leak
  children_.clear();

  // Split nodes into appropriate sizes. The first node will always be n.
  int page_size = txn->page_size();
  auto nodes = split(page_size);

  for (auto node : nodes) {
    // Add node's page to the freelist if it's not new.
    if (node->pgid_ > 0) {
      txn->free(node->pgid_);
      node->pgid_ = 0;
    }
  }
}

}  // namespace boltdb