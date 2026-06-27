#include <catch2/catch_test_macros.hpp>

// Typical unit under test — a simple accumulator
struct Accumulator {
  int value = 0;
  void add(int n) { value += n; }
  void reset() { value = 0; }
};

TEST_CASE("Accumulator starts at zero") {
  Accumulator a;
  REQUIRE(a.value == 0);
}

TEST_CASE("Accumulator addition") {
  Accumulator a;

  SECTION("single add") {
    a.add(5);
    REQUIRE(a.value == 5);
  }

  SECTION("multiple adds") {
    a.add(3);
    a.add(4);
    REQUIRE(a.value == 7);
  }

  SECTION("negative values") {
    a.add(10);
    a.add(-3);
    REQUIRE(a.value == 7);
  }
}

TEST_CASE("Accumulator reset") {
  Accumulator a;
  a.add(42);
  a.reset();
  REQUIRE(a.value == 0);
}
