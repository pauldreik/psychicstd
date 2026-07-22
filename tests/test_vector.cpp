#include "psyassert.h"
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

struct operator_hijacker {
  template <typename T> friend void operator&(T&&) = delete;
  template <typename T, typename U> friend void operator,(T&&, U&&) = delete;
};

template <typename T> struct fancy_pointer {
  using element_type = T;
  template <typename U> using rebind = fancy_pointer<U>;
};

template <typename T> struct fancy_allocator {
  using value_type = T;
  using pointer = fancy_pointer<T>;
  using const_pointer = fancy_pointer<const T>;
};

using fancy_vector = std::vector<int, fancy_allocator<int>>;
static_assert(__is_same(fancy_vector::pointer, fancy_pointer<int>));
static_assert(__is_same(fancy_vector::const_pointer, fancy_pointer<const int>));

int main() {
  std::vector<char> zeroes(32);
  for (char c : zeroes)
    psyassert(c == '\0');

  std::vector<unsigned char> bytes(32, 0xa5);
  for (unsigned char c : bytes)
    psyassert(c == 0xa5);

  std::vector<int> v;
  v.push_back(42);
  psyassert(v[0] == 42);

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

  std::vector<operator_hijacker> hijackers;
  std::vector<operator_hijacker> other_hijackers;
  hijackers = other_hijackers;
  hijackers = static_cast<decltype(other_hijackers)&&>(other_hijackers);
  hijackers.insert(hijackers.begin(), hijackers.begin(), hijackers.end());
}
