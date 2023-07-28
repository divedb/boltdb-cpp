#define CATCH_CONFIG_MAIN

#include "boltdb/util/slice.hpp"

#include <catch2/catch.hpp>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

using namespace boltdb;

TEST_CASE("Default constructor", "[ByteSlice]") {
  ByteSlice slice;

  REQUIRE(slice.ref_count() == 1);
  REQUIRE(slice.to_string() == "");
}

TEST_CASE("Construct from char* and string", "[ByteSlice]") {
  std::string expected = "hello world";
  ByteSlice slice1(expected);

  REQUIRE(slice1.ref_count() == 1);
  REQUIRE(slice1.to_string() == expected);

  ByteSlice slice2(expected.c_str(), 5);

  REQUIRE(slice2.ref_count() == 1);
  REQUIRE(slice2.to_string() == expected.substr(0, 5));

  std::string str(expected);
  ByteSlice slice3(str);

  REQUIRE(slice3.ref_count() == 1);
  REQUIRE(slice3.to_string() == expected);
}

TEST_CASE("Copy constructor and assignment", "[ByteSlice]") {
  std::string str1 = "hello world";
  ByteSlice slice1(str1);
  ByteSlice slice2(slice1);

  REQUIRE(slice1.ref_count() == 2);
  REQUIRE(slice2.ref_count() == 2);
  REQUIRE(slice1.to_string() == str1);
  REQUIRE(slice2.to_string() == str1);

  std::string str2 = "world hello";
  ByteSlice slice3(str2);

  REQUIRE(slice3.ref_count() == 1);
  REQUIRE(slice3.to_string() == str2);

  // Slice3 now should point to "hello world".
  slice3 = slice1;

  REQUIRE(slice1.ref_count() == 3);
  REQUIRE(slice2.ref_count() == 3);
  REQUIRE(slice3.ref_count() == 3);
  REQUIRE(slice3.to_string() == str1);

  slice1 = slice2;
  REQUIRE(slice1.ref_count() == 3);
  REQUIRE(slice2.ref_count() == 3);
  REQUIRE(slice3.ref_count() == 3);

  // Self assignment.
  slice1 = slice1;
  REQUIRE(slice1.ref_count() == 3);
  REQUIRE(slice2.ref_count() == 3);
  REQUIRE(slice3.ref_count() == 3);
}

TEST_CASE("Move constructor and assignment", "[Byte Slice]") {
  std::string str1 = "hello world";
  ByteSlice slice1(str1);
  ByteSlice slice2(std::move(slice1));

  REQUIRE(bool(slice1) == false);
  REQUIRE(slice1.ref_count() == 0);
  REQUIRE(slice1.to_string() == "");

  REQUIRE(slice2.ref_count() == 1);
  REQUIRE(slice2.to_string() == str1);
}