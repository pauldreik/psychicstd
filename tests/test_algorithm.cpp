#include "psyassert.h"
#include <algorithm>
#include <vector>

struct move_only {
  int value;
  explicit move_only(int v) : value(v) {}
  move_only(const move_only&) = delete;
  move_only& operator=(const move_only&) = delete;
  move_only(move_only&&) = default;
  move_only& operator=(move_only&&) = default;
  friend bool operator<(const move_only& a, const move_only& b) {
    return a.value < b.value;
  }
};

constexpr bool constexpr_nth_element() {
  int values[] = {5, 1, 4, 2, 3};
  std::nth_element(values, values + 2, values + 5);
  return values[2] == 3 && values[0] <= 3 && values[1] <= 3 && values[3] >= 3 &&
         values[4] >= 3;
}

static_assert(constexpr_nth_element());

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

  std::vector<int> duplicates(64, 7);
  std::nth_element(duplicates.begin(), duplicates.begin() + 32,
                   duplicates.end());
  psyassert(duplicates[32] == 7);

  std::vector<int> permutation = {0, 1, 2, 3, 4};
  do {
    for (std::size_t nth = 0; nth < permutation.size(); ++nth) {
      auto selected = permutation;
      std::nth_element(selected.begin(), selected.begin() + nth,
                       selected.end());
      psyassert(selected[nth] == static_cast<int>(nth));
      for (std::size_t i = 0; i < nth; ++i)
        psyassert(selected[i] <= selected[nth]);
      for (std::size_t i = nth + 1; i < selected.size(); ++i)
        psyassert(selected[i] >= selected[nth]);
    }
  } while (std::next_permutation(permutation.begin(), permutation.end()));

  std::vector<int> descending = {1, 5, 2, 4, 3};
  std::nth_element(descending.begin(), descending.begin() + 2, descending.end(),
                   [](int x, int y) { return x > y; });
  psyassert(descending[2] == 3);

  std::vector<int> src = {5, 1, 4, 2, 3};
  std::vector<int> dst(3);
  auto out =
      std::partial_sort_copy(src.begin(), src.end(), dst.begin(), dst.end());
  psyassert(out == dst.end());
  psyassert(dst[0] == 1);
  psyassert(dst[1] == 2);
  psyassert(dst[2] == 3);

  std::vector<move_only> movable;
  movable.emplace_back(5);
  movable.emplace_back(1);
  movable.emplace_back(4);
  movable.emplace_back(2);
  movable.emplace_back(3);
  std::nth_element(movable.begin(), movable.begin() + 2, movable.end());
  psyassert(movable[2].value == 3);
}
