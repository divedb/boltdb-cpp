#include <gtest/gtest.h>

#include <cstddef>
#include <vector>

#include "boltdb/alloc/memory_pool.hpp"
#include "boltdb/util/slice.hpp"

using namespace boltdb;

// Return a vector of a pair of <pointer to block, block size>.
std::vector<std::pair<Byte*, std::size_t>> allocate_blocks(
    MemoryPool& pool, const std::vector<std::size_t>& block_sizes) {
  std::vector<std::pair<Byte*, std::size_t>> byte_ptrs(block_sizes.size());

  std::transform(block_sizes.begin(), block_sizes.end(), byte_ptrs.begin(),
                 [&pool](std::size_t nbytes) {
                   return std::make_pair(pool.allocate(nbytes), nbytes);
                 });

  return byte_ptrs;
}

void memory_pool_test(const std::vector<std::size_t>& block_sizes) {
  auto& pool = MemoryPool::instance();
  std::size_t expected =
      std::accumulate(block_sizes.begin(), block_sizes.end(), 0);

  std::vector<std::pair<Byte*, std::size_t>> byte_ptrs =
      allocate_blocks(pool, block_sizes);

  REQUIRE(pool.bytes_outstanding() == expected);

  std::random_shuffle(byte_ptrs.begin(), byte_ptrs.end());

  while (!byte_ptrs.empty()) {
    auto [ptr, nbytes] = byte_ptrs.back();
    byte_ptrs.pop_back();

    pool.deallocate(ptr, nbytes);
    expected -= nbytes;

    REQUIRE(pool.bytes_outstanding() == expected);
  }

  REQUIRE(pool.bytes_outstanding() == 0);
}

TEST(MemoryPoolTest, SmallBlockSize) {
  std::vector<std::size_t> block_sizes = {1, 3, 5, 7, 9};
  memory_pool_test(block_sizes);
}

TEST(MemoryPoolTest, LargeBlockSize) {
  std::vector<std::size_t> block_sizes = {1 << 10, 1 << 11, 1 << 12, 1 << 13,
                                          1 << 15};
  memory_pool_test(block_sizes);
}

TEST(MemoryPoolTest, MixedBlockSize) {
  std::vector<std::size_t> block_sizes = {1 << 10, 1, 1 << 12, 3, 1 << 15};
  memory_pool_test(block_sizes);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}