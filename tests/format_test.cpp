#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include <string>

#include "boltdb/util/util.hpp"

using namespace boltdb;

TEST_CASE("Number hex format", "[Format]") {
  int n = 42;
  std::string hex = format("%02x", n);

  REQUIRE(hex == "2a");
}