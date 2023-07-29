#define CATCH_CONFIG_MAIN

#include "boltdb/util/ref_count.hpp"

#include <catch2/catch.hpp>

using namespace boltdb;

TEST_CASE("Constructor", "[Reference Count]") {
  RefCount ref1;

  REQUIRE(ref1.count() == 1);

  RefCount ref2(ref1);

  REQUIRE(ref1.count() == 2);
  REQUIRE(ref2.count() == 2);

  RefCount ref3;

  REQUIRE(ref3.count() == 1);

  ref1 = ref3;

  REQUIRE(ref1.count() == 2);
  REQUIRE(ref3.count() == 2);
  REQUIRE(ref2.count() == 1);
}