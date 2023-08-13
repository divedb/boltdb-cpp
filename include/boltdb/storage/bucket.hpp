#ifndef BOLTDB_CPP_STORAGE_BUCKET_HPP_
#define BOLTDB_CPP_STORAGE_BUCKET_HPP_

#include <map>
#include <string>

#include "boltdb/util/common.hpp"

namespace boltdb {

class Page;
class Node;
class Transaction;

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
  static constexpr const int kMaxKeySize = 32768;

  // MaxValueSize is the maximum length of a value, in bytes.
  // TODO(gc): why subtract 2
  static constexpr const int kMaxValueSize = (1 << 31) - 2;

  static constexpr const f32 kMinFillPercent = 0.1;
  static constexpr const f32 kMaxFillPercent = 1.0;

  // DefaultFillPercent is the percentage that split pages are filled.
  // This value can be changed by setting Bucket.FillPercent.
  static constexpr const f32 kDefaultFillPercent = 0.5;

  Bucket(Transaction* txn);

 private:
  BucketMeta _bucket_meta;
  Transaction* _txn;  // The associated transaction
  std::map<std::string, Bucket*> _sub_buckets_cache;  // Subbucket cache
  Page* _page;                                        // Inline page reference
  Node* _root_node;                     // Materialized node for the root page
  std::map<PageID, Node*> _node_cache;  // Node cache

  // Sets the threshold for filling nodes when they split. By default,
  // the bucket will fill to 50% but it can be useful to increase this
  // amount if you know that your write workloads are mostly append-only.
  //
  // This is non-persisted across transactions so it must be set in every
  // transaction.
  f64 _fill_percent;
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_STORAGE_BUCKET_HPP_