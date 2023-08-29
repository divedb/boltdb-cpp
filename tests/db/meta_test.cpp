#include <gtest/gtest.h>

#include <cstdint>
#include <string>

#include "boltdb/db/db.hpp"
#include "boltdb/os/darwin.hpp"
#include "boltdb/util/slice.hpp"

using namespace boltdb;

TEST(MetaTest, SerializeAndDeserialize) {
  u32 page_size = OS::getpagesize();

  Meta meta{.magic = DB::kMagic,
            .version = DB::kVersion,
            .page_size = page_size,
            .flags = 0,
            .root = {.root = 3, .sequence = 0},
            .freelist = 2,
            .pgid = 0,
            .txid = 0};
  meta.compute_checksum();

  ByteSlice slice = Meta::serialize(meta);
  Meta new_meta = Meta::deserialize(slice);

  EXPECT_TRUE(new_meta.equals(meta));
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}