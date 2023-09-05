#ifndef BOLTDB_CPP_STORAGE_BUCKET_HPP_
#define BOLTDB_CPP_STORAGE_BUCKET_HPP_

#include <map>
#include <string>

#include "boltdb/util/common.hpp"
#include "boltdb/util/types.hpp"

namespace boltdb {

class Page;
class Node;
class Txn;

// Represents the on-file representation of a bucket.
// This is stored as the "value" of a bucket key. If the bucket is small enough,
// then its root page can be stored inline in the value, after the bucket
// header. In the case of inline buckets, the "root" will be 0.
struct BucketMeta {
 public:
  PageID root;   // Page id of the bucket's root-level page
  u64 sequence;  // Monotonically incrementing, used by NextSequence()
};

// Bucket represents a collection of key/value pairs inside the database.
class Bucket {
 public:
  // MaxKeySize is the maximum length of a key, in bytes. (1 << 15)
  static constexpr const u16 kMaxKeySize = 32768;

  // MaxValueSize is the maximum length of a value, in bytes.
  // TODO(gc): why subtract 2
  static constexpr const u32 kMaxValueSize = (1U << 31) - 2;

  static constexpr const f32 kMinFillPercent = 0.1;
  static constexpr const f32 kMaxFillPercent = 1.0;

  // DefaultFillPercent is the percentage that split pages are filled.
  // This value can be changed by setting Bucket.FillPercent.
  static constexpr const f32 kDefaultFillPercent = 0.5;

  Bucket(Txn* txn);

  // Creates a node from the specified page ID and attaches it to the given
  // parent.
  // Don't free this node, the bucket is responsible for keeping the memory
  // clean.
  Node* node(PageID pgid, Node* parent);

  const Txn txn() const;

 private:
  BucketMeta bucket_meta_;
  Txn* txn_;  // The associated transaction
  std::map<std::string, Bucket*> sub_buckets_cache_;  // Subbucket cache
  Page* page_;                                        // Inline page reference
  Node* root_node_;                     // Materialized node for the root page
  std::map<PageID, Node*> node_cache_;  // Node cache

  // Sets the threshold for filling nodes when they split. By default,
  // the bucket will fill to 50% but it can be useful to increase this
  // amount if you know that your write workloads are mostly append-only.
  //
  // This is non-persisted across transactions so it must be set in every
  // transaction.
  f64 fill_percent_;
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_STORAGE_BUCKET_HPP_