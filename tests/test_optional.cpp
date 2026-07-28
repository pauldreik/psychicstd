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
  std::optional<double> source = 4.5;
  std::optional<float> converted = source;
  psyassert(converted && *converted == 4.5F);
  std::optional<double> no_source;
  std::optional<float> no_converted = no_source;
  psyassert(!no_converted);

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
  std::optional<multiple_arguments> in_place(std::in_place, 4, 5);
  psyassert(in_place->value == 9);

  std::optional<int> empty;
  bool bad_access = false;
  try {
    (void)empty.value();
  } catch (const std::bad_optional_access& error) {
    bad_access = error.what()[0] != '\0';
  }
  psyassert(bad_access);

  std::optional<int> low = 1;
  std::optional<int> high = 2;
  psyassert(empty < low);
  psyassert(low < high);
  psyassert(high > low);
  psyassert(low <= low);
  psyassert(high >= low);
  psyassert(empty < 0);
  psyassert(!(empty > 0));
  psyassert(high > 1);
  psyassert(1 < high);
  psyassert(low <= 1);
  psyassert(1 <= low);
  psyassert(high >= 2);
  psyassert(2 >= high);

  std::optional<aggregate> aggregate_value;
  std::optional<aggregate> constructed_aggregate{{1, 2}};
  std::optional<aggregate> single_braced{{1}};
  psyassert(single_braced->first == 1);
  psyassert(constructed_aggregate->second == 2);
  aggregate_value = {3, 4};
  psyassert(aggregate_value->first == 3);
  aggregate_value = {5, 6};
  psyassert(aggregate_value->second == 6);
}
