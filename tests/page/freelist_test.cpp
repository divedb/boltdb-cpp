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

TEST(FreeListTest, AllocateContiguous) {
  vector<PageID> pgids{3, 4, 5, 6, 7, 9, 12, 13, 18};
  FreeList freelist(pgids);

  // 3, 4, 5 are freed.
  PageID pid = freelist.allocate_contiguous(3);
  EXPECT_EQ(3, pid);

  // 6 is freed.
  pid = freelist.allocate_contiguous(1);
  EXPECT_EQ(6, pid);

  pid = freelist.allocate_contiguous(3);
  EXPECT_EQ(0, pid);

  // 12, 13 are freed
  pid = freelist.allocate_contiguous(2);
  EXPECT_EQ(12, pid);

  // 7 is freed.
  pid = freelist.allocate_contiguous(1);
  EXPECT_EQ(7, pid);

  // 9, 18 are remained.
  pid = freelist.allocate_contiguous(0);
  EXPECT_EQ(0, pid);

  pid = freelist.allocate_contiguous(0);
  EXPECT_EQ(0, pid);

  EXPECT_FALSE(freelist.is_freed(9));
  EXPECT_FALSE(freelist.is_freed(18));

  pid = freelist.allocate_contiguous(1);
  EXPECT_EQ(9, pid);

  pid = freelist.allocate_contiguous(1);
  EXPECT_EQ(18, pid);

  pid = freelist.allocate_contiguous(1);
  EXPECT_EQ(0, pid);

  for (auto id : pgids) {
    EXPECT_FALSE(freelist.is_freed(id));
  }
}

TEST(FreeListTest, Read) {
  // Create a page.
  Page page = make_page(100);
  page.set_flag(kFreeList);
  page.set_count(2);

  // Insert 2 page ids.
  auto ids = reinterpret_cast<PageID*>(page.skip_page_header());
  ids[0] = 23;
  ids[1] = 50;

  // Deserialize page into a freelist.
  FreeList freelist;
  freelist.read_from(page);

  // Ensure that there are two page ids in the freelist.
  EXPECT_TRUE(freelist.is_freed(23));
  EXPECT_TRUE(freelist.is_freed(50));
}

TEST(FreeListTest, Write) {
  std::vector<PageID> ids = {12, 39};
  FreeList freelist(ids);

  std::map<TxnID, std::vector<Page>> pending = {
      {100, {make_page(28), make_page(11)}}, {101, {make_page(3)}}};

  for (auto&& [txn_id, page_vec] : pending) {
    for (auto&& page : page_vec) {
      freelist.free(txn_id, &page);
    }
  }

  Page page = make_page(42);
  Status status = freelist.write_to(page);

  page.hexdump(std::cout);

  EXPECT_TRUE(status.ok());

  // Read the page back out.
  FreeList new_freelist;
  new_freelist.read_from(page);

  // Ensure that the freelist is correct.
  // All pages should be present and in reverse order.
  for (auto&& [txn_id, page_vec] : pending) {
    for (auto&& page : page_vec) {
      EXPECT_TRUE(new_freelist.is_freed(page.id()));
    }
  }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}