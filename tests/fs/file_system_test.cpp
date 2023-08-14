#include "boltdb/fs/file_system.hpp"

#include <gtest/gtest.h>

#include "boltdb/util/status.hpp"

TEST(FileSystemTest, CreateOK) {
  const char* path = "/tmp/boltdb.txt";
  auto handle = boltdb::FileSystem::create(path);

  EXPECT_TRUE(handle != nullptr);
  EXPECT_TRUE(boltdb::FileSystem::exists(*handle));
}

TEST(FileSystemTest, RemoveOK) {
  const char* path = "/tmp/boltdb.txt";
  auto handle = boltdb::FileSystem::open(path, O_RDONLY, 0);

  if (handle == nullptr) {
    handle = boltdb::FileSystem::create(path);

    EXPECT_TRUE(handle != nullptr);
  }

  boltdb::Status status = boltdb::FileSystem::remove(*handle);
  EXPECT_EQ(boltdb::StatusType::kStatusOK, status.status_type());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}