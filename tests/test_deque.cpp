#include <cassert>
#include <deque>

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
  std::deque<int> d;
  d.push_back(1);
  assert(d.front() == 1);

  int n = 1;
  std::deque<value> values;
  values.emplace_back(n);
  values.emplace_front(static_cast<int&&>(n));
  values.insert(values.begin() + 1, value(n));
  assert(values.size() == 3);
  assert(values[0].n == 11);
  assert(values[1].n == 1);
  assert(values[2].n == 1);

  std::deque<value> moved(static_cast<decltype(values)&&>(values));
  assert(moved.size() == 3);
  std::deque<value> other;
  other.swap(moved);
  assert(other.size() == 3);
}
