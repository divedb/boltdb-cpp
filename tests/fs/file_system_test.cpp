#include "boltdb/fs/file_system.hpp"

#include <gtest/gtest.h>

TEST(FileSystemTest, CreateOK) {
  const char* path = "/tmp/boltdb.txt";
  auto handle = boltdb::FileSystem::create(path);

  EXPECT_TRUE(handle != nullptr);
  EXPECT_TRUE(boltdb::FileSystem::exists(*handle));
}