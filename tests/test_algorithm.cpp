#include "psyassert.h"
#include <algorithm>
#include <vector>

int main() {
  std::vector<int> v = {3, 1, 2};
  std::sort(v.begin(), v.end());
  psyassert(v[0] == 1);

  std::vector<int> a = {1, 2, 3, 4};
  std::vector<int> b = {1, 2, 9, 4};
  auto m = std::mismatch(a.begin(), a.end(), b.begin(), b.end());
  psyassert(*m.first == 3);
  psyassert(*m.second == 9);

  std::vector<int> needles = {7, 3};
  psyassert(std::find_first_of(a.begin(), a.end(), needles.begin(),
                               needles.end()) == a.begin() + 2);
  needles = {7, 8};
  psyassert(std::find_first_of(a.begin(), a.end(), needles.begin(),
                               needles.end()) == a.end());
}
