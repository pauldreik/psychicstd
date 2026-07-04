#include <vector>

std::vector<int> evens(const std::vector<int>& v) {
  std::vector<int> result;
  for (int x : v) {
    if (x % 2 == 0) {
      result.push_back(x);
    }
  }
  return result;
}
