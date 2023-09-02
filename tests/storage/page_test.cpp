#include "boltdb/storage/page.hpp"

#include <gtest/gtest.h>

#include <vector>

constexpr static const int kMockPageSize = 4096;

using namespace boltdb;

class TestCase {
 public:
  PageFlag flag;
  std::string type;
};

TEST(PageTest, PageType) {
  const std::vector<TestCase> test_cases{{PageFlag::kBranch, "branch"},
                                         {PageFlag::kLeaf, "leaf"},
                                         {PageFlag::kMeta, "meta"},
                                         {PageFlag::kFreeList, "freelist"}};

  for (auto&& tc : test_cases) {
    Page page(tc.flag, 0, kMockPageSize);

    EXPECT_EQ(tc.type, page.type());
  }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}