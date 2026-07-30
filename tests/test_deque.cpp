#include "psyassert.h"
#include <deque>
#include <iterator>

struct value {
  int n;

  explicit value(int& v) : n(v) {}
  explicit value(int&& v) : n(v + 10) {}
  value(const value&) = delete;
  value& operator=(const value&) = delete;
  value(value&& o) noexcept : n(o.n) { o.n = 0; }
  value& operator=(value&& o) noexcept {
    n = o.n;
    o.n = 0;
    return *this;
  }
};

struct throwing_move_only {
  int value;
  explicit throwing_move_only(int v) : value(v) {}
  throwing_move_only(const throwing_move_only&) = delete;
  throwing_move_only& operator=(const throwing_move_only&) = delete;
  throwing_move_only(throwing_move_only&& other) noexcept(false)
      : value(other.value) {
    other.value = 0;
  }
  throwing_move_only& operator=(throwing_move_only&& other) noexcept(false) {
    value = other.value;
    other.value = 0;
    return *this;
  }
};

int main() {
  std::deque<int> empty;
  empty.insert(empty.end(), 1);
  psyassert(empty.size() == 1);
  psyassert(empty.front() == 1);
  auto out = std::inserter(empty, empty.end());
  *out++ = 2;
  *out++ = 3;
  psyassert(empty.size() == 3);
  psyassert(empty[1] == 2);
  psyassert(empty[2] == 3);

  std::deque<int> d;
  d.push_back(1);
  psyassert(d.front() == 1);

  int n = 1;
  std::deque<value> values;
  values.emplace_back(n);
  values.emplace_front(static_cast<int&&>(n));
  values.insert(values.begin() + 1, value(n));
  psyassert(values.size() == 3);
  psyassert(values[0].n == 11);
  psyassert(values[1].n == 1);
  psyassert(values[2].n == 1);

  std::deque<value> moved(static_cast<decltype(values)&&>(values));
  psyassert(moved.size() == 3);
  std::deque<value> other;
  other.swap(moved);
  psyassert(other.size() == 3);

  std::deque<int> erased{1, 2, 3, 2, 4};
  psyassert(std::erase(erased, 2) == 2);
  psyassert((erased == std::deque<int>{1, 3, 4}));

  int predicate_calls = 0;
  psyassert(std::erase_if(erased, [&](int item) {
              ++predicate_calls;
              return item != 3;
            }) == 2);
  psyassert(predicate_calls == 3);
  psyassert((erased == std::deque<int>{3}));

  std::deque<value> erased_values;
  erased_values.emplace_back(n);
  erased_values.emplace_back(n);
  erased_values.emplace_back(n);
  erased_values[1].n = 2;
  psyassert(std::erase_if(erased_values,
                          [](const value& item) { return item.n == 1; }) == 2);
  psyassert(erased_values.size() == 1);
  psyassert(erased_values[0].n == 2);

  std::deque<throwing_move_only> throwing_moves;
  for (int i = 1; i <= 8; ++i)
    throwing_moves.emplace_back(i);
  for (int i = 0; i < 8; ++i)
    psyassert(throwing_moves[static_cast<std::size_t>(i)].value == i + 1);
}
