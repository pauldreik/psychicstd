#include <algorithm>
#include <atomic>
#include <cassert>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

struct alignas(16) wide_value {
  int first;
  int second;
  int third;
  int fourth;
};

int main() {
  // I/O — verifies <iostream> + <ostream> + streambuf work
  std::cout << "hello from psychicstd!\n";

  // Basic string operations
  std::string s = "psychicstd";
  assert(s.size() == 10);
  assert(s == "psychicstd");
  s += " rocks";
  assert(s == "psychicstd rocks");

  // Vector with sort
  std::vector<int> v = {5, 3, 1, 4, 2};
  std::sort(v.begin(), v.end());
  assert(v == std::vector<int>({1, 2, 3, 4, 5}));

  // Numeric
  assert(std::gcd(12, 8) == 4);

  // Generic atomics may use the out-of-line libatomic runtime.
  std::atomic<wide_value> atomic_value(wide_value{1, 2, 3, 4});
  wide_value expected{1, 2, 3, 4};
  assert(
      atomic_value.compare_exchange_strong(expected, wide_value{5, 6, 7, 8}));

  std::cout << "all checks passed\n";
  return 0;
}
