#include "psyassert.h"
#include <algorithm>
#include <initializer_list>
#include <map>
#include <ranges>
#include <tuple>
#include <utility>
#include <vector>

namespace custom {
inline int swaps;

struct iterator {
  int* p;
  int& operator*() const { return *p; }
};

int iter_move(iterator i) { return *i + 10; }
void iter_swap(iterator a, iterator b) {
  ++swaps;
  int tmp = *a;
  *a = *b;
  *b = tmp;
}
}

template <typename Range>
concept can_make_subrange = requires(Range&& range) {
  std::ranges::subrange(static_cast<Range&&>(range));
};

static_assert(can_make_subrange<std::vector<int>&>);
static_assert(!can_make_subrange<std::vector<int>>);

struct identity_transform {
  constexpr int operator()(int value) const { return value; }
};

template <typename Range>
concept can_pipe_transform = requires(Range&& range) {
  static_cast<Range&&>(range) | std::views::transform(identity_transform{});
};

template <typename Range>
concept can_call_transform = requires(Range&& range) {
  std::views::transform(static_cast<Range&&>(range), identity_transform{});
};

static_assert(can_pipe_transform<std::vector<int>&>);
static_assert(can_pipe_transform<std::vector<int>>);
static_assert(!can_pipe_transform<std::initializer_list<int>>);
static_assert(!can_call_transform<std::initializer_list<int>>);

int main() {
  std::vector<int> v = {1, 2, 3};
  psyassert(*std::ranges::begin(v) == 1);
  psyassert(std::ranges::count(v, 2) == 1);
  psyassert(std::ranges::count(v.begin(), v.end(), 4) == 0);
  psyassert(std::ranges::all_of(v, [](int value) { return value > 0; }));
  std::ranges::subrange tail(v.begin() + 1, v.end());
  psyassert(*tail.begin() == 2);
  std::ranges::subrange whole(v);
  psyassert(*whole.begin() == 1);

  auto doubled = v | std::views::transform([](int value) { return value * 2; });
  auto transformed = doubled.begin();
  psyassert(*transformed == 2);
  ++transformed;
  psyassert(*transformed == 4);
  auto direct = std::views::transform(v, [](int value) { return value + 1; });
  psyassert(*direct.begin() == 2);
  auto owned = std::vector<int>{3, 4} |
               std::views::transform([](int value) { return value + 1; });
  psyassert(*owned.begin() == 4);

  std::map<int, int> map{{1, 2}, {3, 4}};
  auto map_keys = std::views::keys(map);
  auto map_values = std::views::values(map);
  psyassert(*map_keys.begin() == 1);
  psyassert(*map_values.begin() == 2);
  std::vector<std::pair<int, int>> pairs{{5, 6}};
  psyassert(*std::views::keys(pairs).begin() == 5);
  psyassert(*std::views::values(pairs).begin() == 6);
  std::vector<std::tuple<int, int>> tuples{{7, 8}};
  psyassert(*std::views::keys(tuples).begin() == 7);
  psyassert(*std::views::values(tuples).begin() == 8);

  int a = 4;
  int b = 5;
  std::ranges::iter_swap(&a, &b);
  psyassert(a == 5 && b == 4);
  psyassert(std::ranges::iter_move(&a) == 5);

  custom::iterator ai{&a};
  custom::iterator bi{&b};
  psyassert(std::ranges::iter_move(ai) == 15);
  std::ranges::iter_swap(ai, bi);
  psyassert(custom::swaps == 1);
  psyassert(a == 4 && b == 5);
}
