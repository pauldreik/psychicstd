#include <cassert>
#include <fmt/core.h>
#include <string>

int main() {
  auto s = fmt::format("hello {}!", "psychicstd");
  assert(s == "hello psychicstd!");

  auto n = fmt::format("{} + {} = {}", 2, 3, 5);
  assert(n == "2 + 3 = 5");

  // Format with named args
  auto named = fmt::format("value: {value}", fmt::arg("value", 42));
  assert(named == "value: 42");
}
