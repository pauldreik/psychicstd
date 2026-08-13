#include "psyassert.h"
#include <atomic>
#include <list>
#include <type_traits>
#include <vector>

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

struct resize_value {
  static inline int moves = 0;
  int n = 0;

  resize_value() = default;
  resize_value(const resize_value&) = default;
  resize_value& operator=(const resize_value&) = default;
  resize_value(resize_value&& other) noexcept : n(other.n) { ++moves; }
  resize_value& operator=(resize_value&& other) noexcept {
    n = other.n;
    ++moves;
    return *this;
  }
};

int main() {
  int raw[] = {1, 2, 3};
  std::vector deduced_from_pointers(raw, raw + 3);
  static_assert(
      std::is_same_v<decltype(deduced_from_pointers), std::vector<int>>);
  psyassert((deduced_from_pointers == std::vector<int>{1, 2, 3}));

  std::list<double> source{1.5, 2.5, 3.5};
  std::vector deduced_from_iterators(source.begin(), source.end());
  static_assert(
      std::is_same_v<decltype(deduced_from_iterators), std::vector<double>>);
  psyassert((deduced_from_iterators == std::vector<double>{1.5, 2.5, 3.5}));

  std::vector<std::atomic<int>> atomics(2);
  psyassert(atomics[0].load() == 0);

  std::vector<char> zeroes(32);
  for (char c : zeroes)
    psyassert(c == '\0');

  std::vector<unsigned char> bytes(32, 0xa5);
  for (unsigned char c : bytes)
    psyassert(c == 0xa5);

  std::vector<int> v;
  v.erase(v.begin(), v.end());
  v.push_back(42);
  psyassert(v[0] == 42);

  std::vector<bool> bools{true, false, true};
  psyassert(
      std::hash<std::vector<bool>>{}(bools) ==
      std::hash<std::vector<bool>>{}(std::vector<bool>{true, false, true}));
  psyassert(std::erase(bools, true) == 2);
  psyassert((bools == std::vector<bool>{false}));

  int n = 1;
  std::vector<value> values;
  values.emplace_back(n);
  values.emplace_back(static_cast<int&&>(n));
  values.insert(values.begin(), value(n));
  values.emplace(values.begin() + 1, n);
  psyassert(values.size() == 4);
  psyassert(values[0].n == 1);
  psyassert(values[1].n == 1);
  psyassert(values[2].n == 1);
  psyassert(values[3].n == 11);

  std::vector<value> moved(static_cast<decltype(values)&&>(values));
  psyassert(moved.size() == 4);
  std::vector<value> other;
  other.swap(moved);
  psyassert(other.size() == 4);

  std::vector<resize_value> resized;
  for (int i = 0; i < 128; ++i) {
    resized.resize(resized.size() + 1);
    resized.back().n = i;
  }
  psyassert(resize_value::moves < 512);
  for (int i = 0; i < 128; ++i)
    psyassert(resized[i].n == i);

  resize_value::moves = 0;
  resize_value fill;
  fill.n = 42;
  std::vector<resize_value> resized_with_value;
  for (int i = 0; i < 128; ++i)
    resized_with_value.resize(resized_with_value.size() + 1, fill);
  psyassert(resize_value::moves < 512);
  for (const auto& item : resized_with_value)
    psyassert(item.n == 42);

  std::vector<int> erased{1, 2, 3, 2, 4};
  psyassert(std::erase(erased, 2) == 2);
  psyassert((erased == std::vector<int>{1, 3, 4}));

  int predicate_calls = 0;
  psyassert(std::erase_if(erased, [&](int item) {
              ++predicate_calls;
              return item != 3;
            }) == 2);
  psyassert(predicate_calls == 3);
  psyassert((erased == std::vector<int>{3}));

  std::vector<value> erased_values;
  erased_values.emplace_back(n);
  erased_values.emplace_back(n);
  erased_values.emplace_back(n);
  erased_values[1].n = 2;
  psyassert(std::erase_if(erased_values,
                          [](const value& item) { return item.n == 1; }) == 2);
  psyassert(erased_values.size() == 1);
  psyassert(erased_values[0].n == 2);
}
