#include "boltdb/storage/bucket.hpp"

using namespace boltdb;

Bucket::Bucket(Transaction* txn) : _txn(txn) {}

inline Transaction* Bucket::transaction() const { return _txn; }

inline PageID Bucket::root() const { return _bucket_meta.root; }