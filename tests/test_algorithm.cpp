#include <algorithm>
#include <cassert>
#include <vector>

int main() {
  std::vector<int> v = {3, 1, 2};
  std::sort(v.begin(), v.end());
  assert(v[0] == 1);

  std::vector<int> a = {1, 2, 3, 4};
  std::vector<int> b = {1, 2, 9, 4};
  auto m = std::mismatch(a.begin(), a.end(), b.begin(), b.end());
  assert(*m.first == 3);
  assert(*m.second == 9);
}
