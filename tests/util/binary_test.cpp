#include "boltdb/util/slice.hpp"
#include "boltdb/util/binary.hpp"

#include <gtest/gtest.h>
#include <cstdint>
#include <string>

using namespace boltdb;

TEST(BigEndianTest, PutUint) {
  ByteSlice slice;
  u32 x = 0x12345678;

  boltdb::binary::BigEndian::put_uint<u32>(slice, x);

  EXPECT_EQ("12345678", slice.to_string());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}