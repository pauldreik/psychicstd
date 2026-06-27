#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>
#include <numeric>
#include <vector>

TEST_CASE("std::sort") {
  std::vector<int> v = {5, 3, 1, 4, 2};
  std::sort(v.begin(), v.end());
  REQUIRE(v == std::vector<int>{1, 2, 3, 4, 5});
}

TEST_CASE("std::accumulate") {
  std::vector<int> v = {1, 2, 3, 4, 5};
  REQUIRE(std::accumulate(v.begin(), v.end(), 0) == 15);
}

TEST_CASE("std::find") {
  std::vector<int> v = {10, 20, 30, 40};
  auto it = std::find(v.begin(), v.end(), 30);
  REQUIRE(it != v.end());
  REQUIRE(*it == 30);
  REQUIRE(std::find(v.begin(), v.end(), 99) == v.end());
}

TEST_CASE("std::count_if") {
  std::vector<int> v = {1, 2, 3, 4, 5, 6};
  auto evens =
      std::count_if(v.begin(), v.end(), [](int x) { return x % 2 == 0; });
  REQUIRE(evens == 3);
}

TEST_CASE("std::transform") {
  std::vector<int> src = {1, 2, 3, 4};
  std::vector<int> dst(src.size());
  std::transform(src.begin(), src.end(), dst.begin(),
                 [](int x) { return x * x; });
  REQUIRE(dst == std::vector<int>{1, 4, 9, 16});
}
