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

struct stable_value {
  int key;
  int order;
};

struct stable_move_only {
  int key;
  int order;
  stable_move_only(int key_, int order_) : key(key_), order(order_) {}
  stable_move_only(const stable_move_only&) = delete;
  stable_move_only& operator=(const stable_move_only&) = delete;
  stable_move_only(stable_move_only&&) = default;
  stable_move_only& operator=(stable_move_only&&) = default;
};

constexpr bool constexpr_nth_element() {
  int values[] = {5, 1, 4, 2, 3};
  std::nth_element(values, values + 2, values + 5);
  return values[2] == 3 && values[0] <= 3 && values[1] <= 3 && values[3] >= 3 &&
         values[4] >= 3;
}

static_assert(constexpr_nth_element());

int main() {
  {
    int values[] = {1, 2, 3, 4};
    int calls = 0;
    auto pred = [&](int value) {
      ++calls;
      return value % 2 != 0;
    };
    psyassert(!std::is_partitioned(values, values + 4, pred));
    psyassert(calls <= 4);
  }

  {
    int values[] = {4, 2, 2, 1, 4};
    auto result = std::minmax_element(values, values + 5);
    psyassert(result.first == values + 3);
    psyassert(result.second == values + 4);
  }

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

  std::vector<stable_value> stable = {{1, 0}, {0, 1}, {1, 2}, {0, 3}};
  std::stable_sort(stable.begin(), stable.end(),
                   [](const stable_value& a, const stable_value& b) {
                     return a.key < b.key;
                   });
  psyassert(stable[0].order == 1);
  psyassert(stable[1].order == 3);
  psyassert(stable[2].order == 0);
  psyassert(stable[3].order == 2);

  std::vector<stable_move_only> stable_movable;
  for (int i = 0; i < 64; ++i)
    stable_movable.emplace_back(i % 4, i);
  std::stable_sort(stable_movable.begin(), stable_movable.end(),
                   [](const stable_move_only& a, const stable_move_only& b) {
                     return a.key < b.key;
                   });
  for (int i = 0; i < 64; ++i) {
    psyassert(stable_movable[i].key == i / 16);
    psyassert(stable_movable[i].order == (i % 16) * 4 + i / 16);
  }

  std::vector<stable_value> random_stable;
  unsigned state = 1;
  for (int i = 0; i < 257; ++i) {
    state = state * 1664525u + 1013904223u;
    random_stable.push_back({static_cast<int>(state % 11), i});
  }
  std::stable_sort(random_stable.begin(), random_stable.end(),
                   [](const stable_value& a, const stable_value& b) {
                     return a.key < b.key;
                   });
  for (std::size_t i = 1; i < random_stable.size(); ++i) {
    psyassert(random_stable[i - 1].key <= random_stable[i].key);
    if (random_stable[i - 1].key == random_stable[i].key)
      psyassert(random_stable[i - 1].order < random_stable[i].order);
  }

  std::vector<int> large(2048);
  for (int i = 0; i < 2048; ++i)
    large[i] = 2047 - i;
  int comparisons = 0;
  std::stable_sort(large.begin(), large.end(), [&](int x, int y) {
    ++comparisons;
    return x < y;
  });
  psyassert(std::is_sorted(large.begin(), large.end()));
  psyassert(comparisons < 500000);

  std::vector<stable_value> merged = {{0, 0}, {1, 1}, {0, 2}, {1, 3}};
  std::inplace_merge(merged.begin(), merged.begin() + 2, merged.end(),
                     [](const stable_value& a, const stable_value& b) {
                       return a.key < b.key;
                     });
  psyassert(merged[0].order == 0);
  psyassert(merged[1].order == 2);
  psyassert(merged[2].order == 1);
  psyassert(merged[3].order == 3);
}
