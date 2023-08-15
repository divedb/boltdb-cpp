#include <gtest/gtest.h>

#include <string>

#include "boltdb/util/util.hpp"

using namespace boltdb;

TEST(FormatTest, SimpleNumber) {
  int n = 42;
  std::string hex = format("%02x", n);

  EXPECT_TRUE(hex == "2a");
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}