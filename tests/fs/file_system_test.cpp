#include "boltdb/fs/file_system.hpp"

#include <gtest/gtest.h>

TEST(FileSystemTest, CreateOK) {
  const char* path = "/tmp/boltdb.txt";
  auto handle = boltdb::FileSystem::create(path);

  EXPECT_TRUE(handle != nullptr);
  EXPECT_TRUE(boltdb::FileSystem::exists(*handle));
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}