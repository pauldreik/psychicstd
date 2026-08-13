#include "psyassert.h"
#include <algorithm>
#include <memory>
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

struct unique_pointer_less {
  using is_transparent = void;
  bool operator()(const std::unique_ptr<int>& a,
                  const std::unique_ptr<int>& b) const {
    return a < b;
  }
  bool operator()(const std::unique_ptr<int>& a, const int* b) const {
    return a.get() < b;
  }
  bool operator()(const int* a, const std::unique_ptr<int>& b) const {
    return a < b.get();
  }
};

int main() {
  const std::set<int> for_crbegin{1, 2, 3};
  psyassert(std::equal(for_crbegin.crbegin(), for_crbegin.crend(),
                       for_crbegin.rbegin(), for_crbegin.rend()));
  psyassert(*for_crbegin.crbegin() == 3);
  psyassert(for_crbegin.crend() == for_crbegin.rend());

  const std::multiset<int> multiset_for_crbegin{1, 2, 2, 3};
  psyassert(
      std::equal(multiset_for_crbegin.crbegin(), multiset_for_crbegin.crend(),
                 multiset_for_crbegin.rbegin(), multiset_for_crbegin.rend()));
  psyassert(*multiset_for_crbegin.crbegin() == 3);

  static_assert(std::is_nothrow_move_constructible_v<std::set<int>>);
  static_assert(
      !std::is_nothrow_move_constructible_v<std::set<int, throwing_move_less>>);

  std::set<int, std::greater<int>> descending({1, 3, 2}, std::greater<int>{});
  psyassert(*descending.begin() == 3);

  std::set<int> s;
  s.insert(42);
  psyassert(s.count(42) == 1);
  int constructor_values[] = {3, 1, 2};
  std::set<int, std::greater<int>> constructed(
      constructor_values, constructor_values + 3, std::greater<int>{});
  psyassert(*constructed.begin() == 3);
  std::set<int> merge_destination{1, 2};
  std::set<int> merge_source{2, 3};
  merge_destination.merge(merge_source);
  psyassert(merge_destination.contains(3));
  psyassert(merge_source.size() == 1 && merge_source.contains(2));
  merge_destination.merge(std::set<int>{4});
  psyassert(merge_destination.contains(4));

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

  std::set<int> range{0, 1, 2, 3, 4, 5};
  auto after_range = range.erase(range.find(1), range.find(5));
  psyassert(after_range == range.find(5));
  psyassert(range.size() == 2);
  psyassert(*range.begin() == 0);
  psyassert(*--range.end() == 5);

  std::multiset<int> duplicate_range{1, 2, 2, 2, 3};
  auto duplicate_first = duplicate_range.lower_bound(2);
  ++duplicate_first;
  auto after_duplicates =
      duplicate_range.erase(duplicate_first, duplicate_range.lower_bound(3));
  psyassert(after_duplicates == duplicate_range.find(3));
  psyassert(duplicate_range.count(2) == 1);
  psyassert(duplicate_range.erase(2) == 1);
  psyassert(duplicate_range.count(2) == 0);

  int first = 1;
  int second = 2;
  std::set<int*> pointers;
  pointers.insert(&first);
  pointers.insert(&second);
  psyassert(pointers.erase(&first) == 1);
  psyassert(pointers.count(&first) == 0);

  std::set<std::unique_ptr<int>, unique_pointer_less> owned;
  owned.insert(std::make_unique<int>(1));
  int* owned_pointer = owned.begin()->get();
  psyassert(owned.find(owned_pointer) == owned.begin());
}
