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

int main() {
  std::vector<int> v;
  v.push_back(42);
  psyassert(v[0] == 42);

  int n = 1;
  std::vector<value> values;
  values.emplace_back(n);
  values.emplace_back(static_cast<int&&>(n));
  values.insert(values.begin(), value(n));
  psyassert(values.size() == 3);
  psyassert(values[0].n == 1);
  psyassert(values[1].n == 1);
  psyassert(values[2].n == 11);

  std::vector<value> moved(static_cast<decltype(values)&&>(values));
  psyassert(moved.size() == 3);
  std::vector<value> other;
  other.swap(moved);
  psyassert(other.size() == 3);
}
