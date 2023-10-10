#ifndef BOLTDB_CPP_DB_BUCKET_META_HPP_
#define BOLTDB_CPP_DB_BUCKET_META_HPP_

namespace boltdb {

// Represents the on-file representation of a bucket.
// This is stored as the "value" of a bucket key. If the bucket is small enough,
// then its root page can be stored inline in the value, after the bucket
// header. In the case of inline buckets, the "root" will be 0.
struct BucketMeta {
 public:
  PageID root;   // Page id of the bucket's root-level page
  u64 sequence;  // Monotonically incrementing, used by NextSequence()
};

}  // namespace boltdb

#endif  // BOLTDB_CPP_DB_BUCKET_META_HPP_