#include <algorithm>
#include <cassert>
#include <vector>

int main() {
  std::vector<int> v = {3, 1, 2};
  std::sort(v.begin(), v.end());
  assert(v[0] == 1);
}
