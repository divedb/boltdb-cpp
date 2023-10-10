#ifndef BOLTDB_CPP_DB_DB_META_HPP_
#define BOLTDB_CPP_DB_DB_META_HPP_

#include "boltdb/db/bucket_meta.hpp"
#include "boltdb/util/slice.hpp"
#include "boltdb/util/status.hpp"
#include "boltdb/util/types.hpp"

namespace boltdb {

class Meta {
 public:
  // Serialize the given meta object.
  static ByteSlice serialize(const Meta& meta);

  // Deserialize meta from the given slice.
  static Meta deserialize(ByteSlice slice);

  // Write meta information to the specified slice.
  // This does *include* the checksum field.
  void write(ByteSlice& slice) const;

  // Compute checksum.
  u64 sum64() const;

  // Checks the marker bytes and version of the meta page to ensure it matches
  // this binary.
  Status validate() const;

  bool equals(const Meta& other) const;

  u32 magic;
  u32 version;
  u32 page_size;
  u32 flags;
  BucketMeta root;
  PageID freelist;  // Freelist page id
  PageID pgid;      // Meta page id
  TxnID txid;       // Transaction id
  u64 checksum;

 private:
  void write_aux(ByteSlice& slice) const;
};
}  // namespace boltdb

#endif  // BOLTDB_CPP_DB_DB_META_HPP_