#include "boltdb/util/binary.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <vector>

#include "boltdb/util/slice.hpp"

using namespace boltdb;

TEST(BigEndianTest, PutUint) {
  ByteSlice slice(4);
  u32 x = 0x61626364;
  binary::BigEndian::put_uint(slice, x);

  EXPECT_EQ("abcd", slice.to_string());
}

TEST(LittleEndianTest, PutUint) {
  ByteSlice slice(4);
  u32 x = 0x61626364;
  binary::LittleEndian::put_uint(slice, x);

  EXPECT_EQ("dcba", slice.to_string());
}

TEST(BigEndianTest, Uint) {
  ByteSlice slice1("\x61\x62");
  u16 x1 = binary::BigEndian::uint<u16>(slice1);

  EXPECT_EQ(0x6162, x1);

  ByteSlice slice2({0x61, 0x62, 0x63, 0x00});
  u32 x2 = binary::BigEndian::uint<u32>(slice2);

  EXPECT_EQ(0x61626300, x2);

  ByteSlice slice3({0x61, 0x62, 0x63, 0x00, 0x00, 0x00, 0x00, 0x00});
  u64 x3 = binary::BigEndian::uint<u64>(slice3);

  EXPECT_EQ(0x6162630000000000, x3);
}

TEST(LittleEndianTest, Uint) {
  ByteSlice slice1("\x61\x62");
  u16 x1 = binary::LittleEndian::uint<u16>(slice1);

  EXPECT_EQ(0x6261, x1);

  ByteSlice slice2({0x61, 0x62, 0x63, 0x00});
  u32 x2 = binary::LittleEndian::uint<u32>(slice2);

  EXPECT_EQ(0x00636261, x2);

  ByteSlice slice3({0x61, 0x62, 0x63, 0x00, 0x00, 0x00, 0x00, 0x00});
  u64 x3 = binary::LittleEndian::uint<u64>(slice3);

  EXPECT_EQ(0x0000000000636261, x3);
}

TEST(BigEndianTest, AppendVariadicUint) {
  Byte b = 0x0f;
  short s = 0x0d0e;
  int i = 0x12345678;
  long long ll = 0x0102030405060708;
  ByteSlice slice;

  slice = binary::BigEndian::append_variadic_uint(slice, b, s, i, ll);

  auto sz = sizeof(b) + sizeof(s) + sizeof(i) + sizeof(ll);
  EXPECT_EQ(sz, slice.size());

  std::vector<Byte> expect = {0x0f, 0x0d, 0x0e, 0x12, 0x34, 0x56, 0x78, 0x01,
                              0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  for (std::size_t i = 0; i < expect.size(); i++) {
    EXPECT_EQ(expect[i], slice[i]);
  }
}

TEST(LittleEndianTest, AppendVariadicUint) {
  Byte b = 0x0f;
  short s = 0x0d0e;
  int i = 0x12345678;
  long long ll = 0x0102030405060708;
  ByteSlice slice;

  slice = binary::LittleEndian::append_variadic_uint(slice, b, s, i, ll);

  auto sz = sizeof(b) + sizeof(s) + sizeof(i) + sizeof(ll);
  EXPECT_EQ(sz, slice.size());

  std::vector<Byte> expect = {0x0f, 0x0e, 0x0d, 0x78, 0x56, 0x34, 0x12, 0x08,
                              0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01};
  for (std::size_t i = 0; i < expect.size(); i++) {
    EXPECT_EQ(expect[i], slice[i]);
  }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}