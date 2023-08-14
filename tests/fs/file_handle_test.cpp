#include <gtest/gtest.h>

#include <iostream>

#include "boltdb/fs/file_system.hpp"
#include "boltdb/util/status.hpp"

TEST(FileHandleTest, DoubleFlock) {
  const char* path = "/tmp/boltdb.txt";
  auto handle = boltdb::FileSystem::create(path);

  EXPECT_TRUE(handle != nullptr);

  boltdb::Status status = handle->flock(LOCK_SH, 0);

  EXPECT_EQ(boltdb::StatusType::kStatusOK, status.status_type());

  // Double shared lock.
  status = handle->flock(LOCK_SH, 0);

  EXPECT_EQ(boltdb::StatusType::kStatusOK, status.status_type());

  // Exclusive lock.
  // A shared lock may be `upgraded` to an exclusive lock, and vice versa,
  // simply by specifying the appropriate lock type; this results in the
  // previous lock being released and the new lock applied (possibly after other
  // processes have gained and released the lock).
  status = handle->flock(LOCK_EX, 0);
  EXPECT_EQ(boltdb::StatusType::kStatusOK, status.status_type());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}