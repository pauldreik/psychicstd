#include <catch2/catch_test_macros.hpp>
#include <string>

TEST_CASE("std::string construction") {
  std::string s = "hello";
  REQUIRE(s.size() == 5);
  REQUIRE(s == "hello");
  REQUIRE(s != "world");
}

TEST_CASE("std::string operations") {
  std::string s = "hello";

  SECTION("append") {
    s += " world";
    REQUIRE(s == "hello world");
    REQUIRE(s.size() == 11);
  }

  SECTION("substr") { REQUIRE(s.substr(1, 3) == "ell"); }

  SECTION("find") {
    REQUIRE(s.find('l') == 2);
    REQUIRE(s.find("llo") == 2);
    REQUIRE(s.find('z') == std::string::npos);
  }

  SECTION("empty and clear") {
    REQUIRE(!s.empty());
    s.clear();
    REQUIRE(s.empty());
    REQUIRE(s.size() == 0);
  }
}

TEST_CASE("std::string concatenation") {
  std::string a = "foo";
  std::string b = "bar";
  REQUIRE(a + b == "foobar");
  REQUIRE(a + "baz" == "foobaz");
  REQUIRE("pre" + b == "prebar");
}
