#include "boltdb/storage/page.hpp"

#include <gtest/gtest.h>

#include <vector>
consexpr static const int kMockPageSize = 4096;

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
    Page page(0, tc.flag, kMockPageSi);

    EXPECT_EQ(page.type, page.type());
  }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}