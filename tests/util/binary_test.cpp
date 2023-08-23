#include "boltdb/util/binary.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <string>

#include "boltdb/util/slice.hpp"

using namespace boltdb;

TEST(BigEndianTest, PutUint) {
  ByteSlice slice(4);
  u32 x = 0x61626364;

  binary::BigEndian::put_uint<u32>(slice, x);

  EXPECT_EQ("abcd", slice.to_string());
}

TEST(BigEndianTest, Uint) {
  ByteSlice slice1("\x61\x62");
  u16 x1 = binary::BigEndian::uint<u16>(slice1);

  EXPECT_EQ(0x6162, x1);

  ByteSlice slice2("\x61\x62\x63\x00");
  u32 x2 = binary::BigEndian::uint<u32>(slice2);

  EXPECT_EQ(0x61626300, x2);

  ByteSlice slice3("\x61\x62\x63\x00");
  u64 x3 = binary::BigEndian::uint<u64>(slice3);

  EXPECT_EQ(0x6162630000000000, x3);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}