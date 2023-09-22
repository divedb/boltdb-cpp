#include "boltdb/util/irange.hpp"

#include <gtest/gtest.h>

#include <vector>

using namespace boltdb;
using namespace std;

struct TestCase {
  int first;
  int last;
  int step;
  std::vector<int> expected;
};

void test_aux(const std::vector<TestCase>& test_cases) {
  for (auto&& tc : test_cases) {
    auto irange = make_irange(tc.first, tc.last, tc.step);

    EXPECT_TRUE(std::equal(irange.begin(), irange.end(), tc.expected.begin(),
                           tc.expected.end()));
  }
}

TEST(IRangeTest, PositiveStep) {
  std::vector<TestCase> test_cases = {{1, 10, 1, {1, 2, 3, 4, 5, 6, 7, 8, 9}},
                                      {1, 10, 2, {1, 3, 5, 7, 9}},
                                      {1, 10, 3, {1, 4, 7}},
                                      {1, 10, 4, {1, 5, 9}},
                                      {1, 10, 5, {1, 6}},
                                      {1, 10, 10, {1}}};

  test_aux(test_cases);
}

TEST(IRangeTest, NegativeStep) {
  std::vector<TestCase> test_cases = {
      {-1, -10, -1, {-1, -2, -3, -4, -5, -6, -7, -8, -9}},
      {-1, -10, -2, {-1, -3, -5, -7, -9}},
      {-1, -10, -3, {-1, -4, -7}},
      {-1, -10, -4, {-1, -5, -9}},
      {-1, -10, -5, {-1, -6}},
      {-1, -10, -10, {-1}}};

  test_aux(test_cases);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}