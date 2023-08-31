#include "boltdb/util/slice.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <span>
#include <string>
#include <utility>
#include <vector>

using namespace boltdb;

TEST(ByteSliceTest, DefaultConstructor) {
  ByteSlice slice;

  EXPECT_EQ(1, slice.ref_count());
  EXPECT_EQ("", slice.to_string());
}

TEST(ByteSliceTest, CustomConstructor) {
  std::string expected = "hello world";
  ByteSlice slice1(expected);

  EXPECT_EQ(1, slice1.ref_count());
  EXPECT_EQ(expected, slice1.to_string());

  ByteSlice slice2(expected.c_str(), 5);

  EXPECT_EQ(1, slice2.ref_count());
  EXPECT_EQ(expected.substr(0, 5), slice2.to_string());

  std::string str(expected);
  ByteSlice slice3(str);

  EXPECT_EQ(1, slice3.ref_count());
  EXPECT_EQ(expected, slice3.to_string());
}

TEST(ByteSliceTest, CopyConstructorAndAssignment) {
  std::string str1 = "hello world";
  ByteSlice slice1(str1);
  ByteSlice slice2(slice1);

  EXPECT_EQ(2, slice1.ref_count());
  EXPECT_EQ(2, slice2.ref_count());
  EXPECT_EQ(str1, slice1.to_string());
  EXPECT_EQ(str1, slice2.to_string());

  std::string str2 = "world hello";
  ByteSlice slice3(str2);

  EXPECT_EQ(1, slice3.ref_count());
  EXPECT_EQ(str2, slice3.to_string());

  // Slice3 now should point to "hello world".
  slice3 = slice1;

  EXPECT_EQ(3, slice1.ref_count());
  EXPECT_EQ(3, slice2.ref_count());
  EXPECT_EQ(3, slice3.ref_count());
  EXPECT_EQ(str1, slice3.to_string());

  slice1 = slice2;
  EXPECT_EQ(3, slice1.ref_count());
  EXPECT_EQ(3, slice2.ref_count());
  EXPECT_EQ(3, slice3.ref_count());

  // Self assignment.
  slice1 = slice1;
  EXPECT_EQ(3, slice1.ref_count());
  EXPECT_EQ(3, slice2.ref_count());
  EXPECT_EQ(3, slice3.ref_count());
}

TEST(ByteSliceTest, MoveConstructorAndAssignment) {
  std::string str1 = "hello world";
  ByteSlice slice1(str1);
  ByteSlice slice2(std::move(slice1));

  // Move slice1 to slice2.
  EXPECT_EQ(0, slice1.ref_count());
  EXPECT_EQ("", slice1.to_string());

  EXPECT_EQ(1, slice2.ref_count());
  EXPECT_EQ(str1, slice2.to_string());
}

TEST(ByteSliceTest, Append) {
  ByteSlice slice;

  for (char ch : kAsciiLowercase) {
    slice.append(ch);
  }

  EXPECT_EQ(kAsciiLowercase, slice.to_string());
}

TEST(ByteSliceTest, Span) {
  ByteSlice slice("\x61\x62");

  std::span<Byte> sp = slice.span();

  EXPECT_EQ(0x61, sp[0]);
  EXPECT_EQ(0x62, sp[1]);
}

TEST(ByteSliceTest, ToHex) {
  ByteSlice slice;

  EXPECT_EQ("", slice.to_hex());

  slice.append('a');
  EXPECT_EQ("0x61,", slice.to_hex());

  slice.append('b');
  EXPECT_EQ("0x61,0x62,", slice.to_hex());
}

TEST(ByteSliceTest, RemovePrefix) {
  ByteSlice slice("hello world");

  slice.remove_prefix(5);
  EXPECT_EQ(" world", slice.to_string());

  slice.remove_prefix(6);
  EXPECT_EQ("", slice.to_string());
  EXPECT_EQ(0, slice.size());

  slice = ByteSlice("\x00\x01\x02\x03", 4);
  EXPECT_EQ("0x00,0x01,0x02,0x03,", slice.to_hex());
  EXPECT_EQ(4, slice.size());
}

TEST(ByteSliceTest, LargeString) {
  std::size_t sz = 1000000;
  ByteSlice slice;
  std::string large_string(sz, 'a');

  for (char c : large_string) {
    slice.append(c);
  }

  EXPECT_EQ(large_string, slice.to_string());
  EXPECT_EQ(sz, slice.size());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}