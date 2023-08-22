#include "boltdb/util/slice.hpp"
#include "boltdb/util/binary.hpp"

#include <gtest/gtest.h>
#include <cstdint>
#include <string>

using namespace boltdb;

TEST(BigEndianTest, PutUint) {
  ByteSlice slice(16);
  std::cout << "size = " << slice.size() << std::endl;
  u32 x = 0x12345678;

  binary::BigEndian::put_uint<u32>(slice, x);

  EXPECT_EQ("12345678", slice.to_string());
}

TEST(BigEndianTest, Uint) {
	ByteSlice slice("\x61\x62");

	std::cout << slice.to_string()<< std::endl;

	u16 x = binary::BigEndian::uint<u16>(slice);

	EXPECT_EQ(0x6162, x);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}