#include "boltdb/util/ref_count.hpp"

#include <gtest/gtest.h>

using namespace boltdb;

TEST(RefCountTest, CopyConstructorAndAssignment) {
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

// TEST(RefCountTest, MoveConstructorAndAssignment) {
//   RefCount ref1;
//   RefCount ref2(std::move(ref1));

//   EXPECT_EQ(0, ref1.count());
//   EXPECT_EQ(1, ref2.count());

//   // Move from valid but unspecified state.
//   RefCount ref3;
//   ref3 = ref1;

//   EXPECT_EQ(0, ref1.count());
//   EXPECT_EQ(0, ref3.count());

//   // Again, move from valid state.
//   ref3 = ref2;
//   EXPECT_EQ(0, ref2.count());
//   EXPECT_EQ(1, ref3.count());
// }

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}