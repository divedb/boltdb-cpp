#include "boltdb/storage/page.hpp"

#include <gtest/gtest.h>

#include <vector>

#include "boltdb/db/db.hpp"

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
    Page page(0, tc.flag, kMockPageSize);

    EXPECT_EQ(tc.type, page.type());
  }
}

TEST(PageTest, Meta) {
  Page page(0, PageFlag::kLeaf, kMockPageSize);

  Meta* meta = page.meta();

  meta->magic = 0x1234;
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}