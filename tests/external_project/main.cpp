#include <algorithm>
#include <atomic>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

struct alignas(16) wide_value {
  int first;
  int second;
  int third;
  int fourth;
};

namespace {

void check(bool ok, std::string_view message) {
  if (!ok)
    throw std::runtime_error(std::string(message));
}

} // namespace

int main() {
  // I/O — verifies <iostream> + <ostream> + streambuf work
  std::cout << "hello from psychicstd!\n";

  // Basic string operations
  std::string s = "psychicstd";
  check(s.size() == 10, "unexpected string size");
  check(s == "psychicstd", "unexpected initial string content");
  s += " rocks";
  check(s == "psychicstd rocks", "unexpected appended string content");

  // Vector with sort
  std::vector<int> v = {5, 3, 1, 4, 2};
  std::sort(v.begin(), v.end());
  check(v == std::vector<int>({1, 2, 3, 4, 5}), "unexpected sort result");

  // Numeric
  check(std::gcd(12, 8) == 4, "unexpected gcd");

  // Generic atomics may use the out-of-line libatomic runtime.
  std::atomic<wide_value> atomic_value(wide_value{1, 2, 3, 4});
  wide_value expected{1, 2, 3, 4};
  check(atomic_value.compare_exchange_strong(expected, wide_value{5, 6, 7, 8}),
        "atomic compare_exchange_strong failed");

  std::cout << "all checks passed\n";
  return 0;
}
