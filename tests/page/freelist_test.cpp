#include "boltdb/page/freelist.hpp"

#include <gtest/gtest.h>

#include "boltdb/page/page.hpp"
#include "boltdb/util/irange.hpp"

using namespace boltdb;
using namespace std;

static Page make_page(PageID pgid) { return {pgid, kFreeList, 4096}; }

class SimpleFreeListTest : public ::testing::Test {
 protected:
  PageID pgid{12};
  TxnID txn_id{100};
  FreeList freelist;
};

TEST_F(SimpleFreeListTest, Free) {
  Page page = make_page(pgid);
  freelist.free(txn_id, &page);

  EXPECT_TRUE(freelist.is_pending(txn_id, pgid));
}

TEST_F(SimpleFreeListTest, FreeOverflow) {
  u32 overflow = 3;
  Page page = make_page(pgid);
  page.set_overflow(overflow);
  freelist.free(txn_id, &page);

  for (auto id : make_irange(pgid, pgid + overflow + 1)) {
    EXPECT_TRUE(freelist.is_pending(txn_id, id));
  }
}

TEST(FreeListTest, Release) {
  FreeList freelist;
  Page page1 = make_page(12);
  Page page2 = make_page(9);
  Page page3 = make_page(39);
  page1.set_overflow(1);

  freelist.free(100, &page1);
  freelist.free(100, &page2);
  freelist.free(102, &page3);
  freelist.release(100);
  freelist.release(101);

  std::vector<PageID> expected = {9, 12, 13};
  for (auto id : expected) {
    EXPECT_FALSE(freelist.is_pending(100, id));
    EXPECT_TRUE(freelist.is_freed(id));
  }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}