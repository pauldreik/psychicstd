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
}
