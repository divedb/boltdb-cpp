#include "boltdb/util/options.hpp"

#include <gtest/gtest.h>

TEST(OptionsTest, DefaultConstructor) {
  boltdb::Options options;

  EXPECT_FALSE(options.is_strict_mode());
  EXPECT_TRUE(options.is_no_sync());
  EXPECT_FALSE(options.is_no_grow_sync());
  EXPECT_FALSE(options.is_read_only());

  EXPECT_EQ(O_RDWR, options.open_flag());
  EXPECT_EQ(0, options.timeout());
  EXPECT_EQ(0, options.mmap_flags());
  EXPECT_EQ(0, options.initial_mmap_size());
  EXPECT_EQ(boltdb::Options::kDefaultMaxBatchSize, options.max_batch_size());
  EXPECT_EQ(boltdb::Options::kDefaultMaxBatchDelay, options.max_batch_delay());
  EXPECT_EQ(boltdb::Options::kDefaultAllocSize, options.alloc_size());
}

TEST(OptionsTest, Modifier) {
  boltdb::Options options;
  double timeout = 1.0;
  int mmapflags = 1;
  int initial_mmap_size = 1024;
  int max_batch_size = 4096;
  int max_batch_delay_ms = 5;
  int alloc_size = 1 << 10;

  options.set_timeout(timeout)
      .set_mmap_flags(mmapflags)
      .set_initial_mmap_size(initial_mmap_size)
      .set_max_batch_size(max_batch_size)
      .set_max_batch_delay(max_batch_delay_ms)
      .set_alloc_size(alloc_size);

  EXPECT_EQ(timeout, options.timeout());
  EXPECT_EQ(mmapflags, options.mmap_flags());
  EXPECT_EQ(initial_mmap_size, options.initial_mmap_size());
  EXPECT_EQ(max_batch_size, options.max_batch_size());
  EXPECT_EQ(max_batch_delay_ms, options.max_batch_delay());
  EXPECT_EQ(alloc_size, options.alloc_size());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}