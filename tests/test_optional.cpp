#include "psyassert.h"
#include <optional>

struct move_only {
  int value;

  explicit move_only(int v) : value(v) {}
  move_only(const move_only&) = delete;
  move_only& operator=(const move_only&) = delete;
  move_only(move_only&& o) noexcept : value(o.value) { o.value = 0; }
  move_only& operator=(move_only&& o) noexcept {
    value = o.value;
    o.value = 0;
    return *this;
  }
};

struct multiple_arguments {
  int value;
  multiple_arguments(int a, int b) : value(a + b) {}
};

struct aggregate {
  int first;
  int second;
};

int main() {
  std::optional<int> o = 42;
  psyassert(o.value() == 42);

  std::optional<move_only> a(move_only(7));
  std::optional<move_only> b(static_cast<decltype(a)&&>(a));
  psyassert(b->value == 7);

  a.emplace(8);
  b = static_cast<decltype(a)&&>(a);
  psyassert(b->value == 8);

  auto c = std::make_optional(move_only(9));
  psyassert(c->value == 9);

  auto d = std::make_optional<multiple_arguments>(10, 20);
  psyassert(d->value == 30);

  std::optional<int> empty;
  std::optional<int> low = 1;
  std::optional<int> high = 2;
  psyassert(empty < low);
  psyassert(low < high);
  psyassert(high > low);
  psyassert(low <= low);
  psyassert(high >= low);

  std::optional<aggregate> aggregate_value;
  aggregate_value = {3, 4};
  psyassert(aggregate_value->first == 3);
  aggregate_value = {5, 6};
  psyassert(aggregate_value->second == 6);
}
