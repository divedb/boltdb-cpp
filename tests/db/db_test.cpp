#include "boltdb/db/db.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <string>

#include "boltdb/os/darwin.hpp"
#include "boltdb/util/options.hpp"
#include "boltdb/util/slice.hpp"

using namespace boltdb;

TEST(DBTest, OpenNonExist) {
  DB* db;
  Options options;
  std::string path = "/tmp/nonexist.db";

  Status status = open_db(path, options, &db);

  std::cout << status.error() << std::endl;

  EXPECT_TRUE(status.status_type() == kStatusOK);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}