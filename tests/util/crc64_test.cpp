#include "boltdb/util/crc64.hpp"

#include <gtest/gtest.h>

#include <string>

using namespace boltdb;

TEST(CRC64, SimpleTest) {
  std::string s1 = "abcd";
  std::string s2 = "abcd";
  std::string s3 = "abce";
  std::string s4 = "abcc";

  u64 seed = 0;

  u64 c1 = crc64_be(seed, s1.c_str(), s1.length());
  u64 c2 = crc64_be(seed, s2.c_str(), s2.length());
  u64 c3 = crc64_be(seed, s3.c_str(), s3.length());
  u64 c4 = crc64_be(seed, s4.c_str(), s4.length());

  EXPECT_EQ(c1, c2);
  EXPECT_NE(c1, c3);
  EXPECT_NE(c1, c4);
  EXPECT_NE(c3, c4);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}