#ifndef BOLTDB_CPP_DB_CURSOR_HPP_
#define BOLTDB_CPP_DB_CURSOR_HPP_

#include <vector>

#include "boltdb/util/slice.hpp"

namespace boltdb {

class Bucket;
class Page;
class Node;

// ElemRef represents a reference to an element on a given page/node.
struct ElemRef {
 public:
  ElemRef() = default;
  ElemRef(Page* p, Node* n, int idx = 0) : page(p), node(n), index(idx) {}

  // Return true if the ref is pointing at a leaf page/node otherwise false.
  bool is_leaf() const;

  // Return the number of inodes or page elements.
  int count() const;

  Page* page{};
  Node* node{};
  int index{};
};

// Cursor represents an iterator that can traverse over all key/value pairs in a
// bucket in sorted order. Cursors see nested buckets with value == nil. Cursors
// can be obtained from a transaction and are valid as long as the transaction
// is open.
//
// Keys and values returned from the cursor are only valid for the life of the
// transaction.
//
// Changing data while traversing with a cursor may cause it to be invalidated
// and return unexpected keys and/or values. You must reposition your cursor
// after mutating data.
class Cursor {
 public:
  // Return the bucket that this cursor was created from.
  Bucket* bucket() { return bucket_; }
  const Bucket* bucket() const { return bucket_; }

  // First moves the cursor to the first item in the bucket
  // and returns its key and value.
  // If the bucket is empty then a nil key and value are returned.
  // The returned key and value are only valid for the life of the transaction.
  std::pair<ByteSlice, ByteSlice> first();

 private:
  // Moves the cursor to the first leaf element under the last page in the
  // stack.
  void move_to_leaf();

  Bucket* bucket_;
  std::vector<ElemRef> stack_;
};

}  // namespace boltdb
#endif  // BOLTDB_CPP_DB_CURSOR_HPP_