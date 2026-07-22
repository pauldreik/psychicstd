#include "psyassert.h"
#include <any>

struct value {
  int n;

  explicit value(int v) : n(v) {}
  value(const value&) = default;
  value(value&& o) noexcept : n(o.n) { o.n = 0; }
  value& operator=(const value&) = default;
  value& operator=(value&& o) noexcept {
    n = o.n;
    o.n = 0;
    return *this;
  }
};

struct tracked {
  static int live;
  tracked() { ++live; }
  tracked(const tracked&) { ++live; }
  tracked(tracked&&) noexcept { ++live; }
  ~tracked() { --live; }
};

int tracked::live = 0;

int main() {
  std::any a = 42;
  psyassert(std::any_cast<int>(a) == 42);

  std::any b = value(7);
  std::any c(static_cast<std::any&&>(b));
  psyassert(std::any_cast<value&>(c).n == 7);

  std::any d = value(8);
  swap(c, d);
  psyassert(std::any_cast<value&>(c).n == 8);
  psyassert(std::any_cast<value&>(d).n == 7);

  value moved = std::any_cast<value>(static_cast<std::any&&>(d));
  psyassert(moved.n == 7);

  {
    std::any source = tracked{};
    std::any target;
    target = static_cast<std::any&&>(source);
    psyassert(tracked::live == 1);
  }
  psyassert(tracked::live == 0);

  bool bad_cast = false;
  try {
    (void)std::any_cast<double>(a);
  } catch (const std::bad_any_cast& error) {
    bad_cast = error.what()[0] != '\0';
  }
  psyassert(bad_cast);
}
