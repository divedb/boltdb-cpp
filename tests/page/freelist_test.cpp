#include "boltdb/page/freelist.hpp"

#include <gtest/gtest.h>

#include "boltdb/page/page.hpp"

using namespace boltdb;
using namespace std;

TEST(FreeListTest, Free) {
  FreeList freelist;
  Page page(12, kFreeList, 4096);

  freelist.free(100, &page);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}