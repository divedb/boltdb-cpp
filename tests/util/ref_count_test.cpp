#include "boltdb/util/ref_count.hpp"

#include <gtest/gtest.h>

using namespace boltdb;

TEST(RefCountTest, Constructor) {
  RefCount ref1;

  EXPECT_EQ(1, ref1.count());

  RefCount ref2(ref1);

  EXPECT_EQ(2, ref1.count());
  EXPECT_EQ(2, ref2.count());

  RefCount ref3;

  EXPECT_EQ(1, ref3.count());

  ref1 = ref3;

  EXPECT_EQ(2, ref1.count());
  EXPECT_EQ(2, ref3.count());
  EXPECT_EQ(1, ref2.count());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}