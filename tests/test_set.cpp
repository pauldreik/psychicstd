#include "psyassert.h"
#include <set>

struct counting_less {
  static inline int comparisons = 0;
  bool operator()(int a, int b) const {
    ++comparisons;
    return a < b;
  }
};

struct throwing_move_less {
  throwing_move_less() = default;
  throwing_move_less(const throwing_move_less&) noexcept(false) {}
  throwing_move_less(throwing_move_less&&) noexcept(false) {}
  bool operator()(int a, int b) const { return a < b; }
};

int main() {
  static_assert(std::is_nothrow_move_constructible_v<std::set<int>>);
  static_assert(
      !std::is_nothrow_move_constructible_v<std::set<int, throwing_move_less>>);

  std::set<int, std::greater<int>> descending({1, 3, 2}, std::greater<int>{});
  psyassert(*descending.begin() == 3);

  std::set<int> s;
  s.insert(42);
  psyassert(s.count(42) == 1);

  std::multiset<int, std::greater<int>> multiple(std::greater<int>{});
  int values[] = {1, 3, 2, 3};
  multiple.insert(values, values + 4);
  multiple.insert(multiple.end(), 4);
  psyassert(*multiple.begin() == 4);
  psyassert(multiple.count(3) == 2);
  psyassert(multiple.count(4) == 1);
  psyassert(
      (multiple == std::multiset<int, std::greater<int>>({4, 3, 3, 2, 1})));
  psyassert(!(multiple == std::multiset<int, std::greater<int>>({4, 3, 2, 1})));
  auto stable_multiple_three = multiple.find(3);
  multiple.insert(5);
  multiple.erase(2);
  psyassert(*stable_multiple_three == 3);

  std::set<int, counting_less> logarithmic_erase;
  for (int i = 0; i < 1024; ++i)
    logarithmic_erase.insert(i);
  counting_less::comparisons = 0;
  psyassert(logarithmic_erase.erase(512) == 1);
  psyassert(counting_less::comparisons < 100);

  std::set<int> stable{1, 2, 3};
  auto stable_three = stable.find(3);
  stable.insert(0);
  stable.erase(2);
  psyassert(*stable_three == 3);
  psyassert(--stable.end() == stable_three);

  int first = 1;
  int second = 2;
  std::set<int*> pointers;
  pointers.insert(&first);
  pointers.insert(&second);
  psyassert(pointers.erase(&first) == 1);
  psyassert(pointers.count(&first) == 0);
}
