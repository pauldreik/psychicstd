#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

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

  std::cout << "all checks passed\n";
  return 0;
}
