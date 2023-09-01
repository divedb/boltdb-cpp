#include "boltdb/util/slice.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <span>
#include <string>
#include <utility>
#include <vector>

using namespace boltdb;

class StringTestCase {
 public:
  StringTestCase(const char* input_str, const char* exp_str,
                 const char* exp_hex_str)
      : input_str(input_str),
        exp_str(exp_str),
        exp_hex_str(exp_hex_str),
        size(strlen(exp_str)) {}

  const char* input_str;
  const char* exp_str;
  const char* exp_hex_str;
  ssize_t size;
};

TEST(ByteSliceTest, DefaultConstructor) {
  ByteSlice slice;

  EXPECT_EQ(1, slice.ref_count());
  EXPECT_EQ("", slice.to_string());
}

TEST(ByteSliceTest, CustomConstructorBuildFromCstring) {
  std::vector<StringTestCase> test_cases = {
      {"abc", "abc", "0x61,0x62,0x63,"},
      {"", "", ""},
      {"abc\x00\x61\x62\x63", "abc", "0x61,0x62,0x63,"}};

  for (auto&& ts : test_cases) {
    ByteSlice slice(ts.input_str);

    EXPECT_EQ(1, slice.ref_count());
    EXPECT_EQ(ts.size, slice.size());
    EXPECT_EQ(ts.exp_str, slice.to_string());
    EXPECT_EQ(ts.exp_hex_str, slice.to_hex());
  }
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

TEST(ByteSliceTest, RemovePrefix) {
  ByteSlice slice("hello world");

  slice.remove_prefix(5);
  EXPECT_EQ(" world", slice.to_string());

  slice.remove_prefix(6);
  EXPECT_EQ("", slice.to_string());
  EXPECT_EQ(0, slice.size());

  slice = ByteSlice(std::vector<Byte>{'\x00', '\x01', '\x02', '\x03'});
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