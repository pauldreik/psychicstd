#include "psyassert.h"
#include <compare>
#include <optional>
#include <type_traits>
#include <utility>

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

struct unrelated {};

struct swappable_not_assignable {
  int value;

  explicit swappable_not_assignable(int v) : value(v) {}
  swappable_not_assignable(const swappable_not_assignable&) = delete;
  swappable_not_assignable& operator=(const swappable_not_assignable&) = delete;
  swappable_not_assignable(swappable_not_assignable&&) noexcept = default;
  swappable_not_assignable& operator=(swappable_not_assignable&&) = delete;

  friend void swap(swappable_not_assignable& a,
                   swappable_not_assignable& b) noexcept {
    int value = a.value;
    a.value = b.value;
    b.value = value;
  }
};

inline char comparison_called;
struct comparison_probe {
  int value;
};
bool operator==(comparison_probe a, comparison_probe b) {
  comparison_called = '=';
  return a.value == b.value;
}
bool operator!=(comparison_probe a, comparison_probe b) {
  comparison_called = '!';
  return a.value != b.value;
}
bool operator<(comparison_probe a, comparison_probe b) {
  comparison_called = '<';
  return a.value < b.value;
}
bool operator>(comparison_probe a, comparison_probe b) {
  comparison_called = '>';
  return a.value > b.value;
}
bool operator<=(comparison_probe a, comparison_probe b) {
  comparison_called = 'l';
  return a.value <= b.value;
}
bool operator>=(comparison_probe a, comparison_probe b) {
  comparison_called = 'g';
  return a.value >= b.value;
}

struct spaceship_probe {
  int value;

  friend bool operator==(spaceship_probe, spaceship_probe) = default;
};
std::strong_ordering operator<=>(spaceship_probe a, spaceship_probe b) {
  comparison_called = '3';
  return a.value <=> b.value;
}

template <typename T, typename U>
concept equality_comparable = requires(T value, U other) { value == other; };

static_assert(!equality_comparable<std::optional<int>, unrelated>);
static_assert(!equality_comparable<unrelated, std::optional<int>>);
static_assert(
    std::is_same_v<decltype(*std::declval<std::optional<int>&&>()), int&&>);
static_assert(std::is_move_constructible_v<swappable_not_assignable>);
static_assert(!std::is_move_assignable_v<swappable_not_assignable>);
static_assert(std::is_swappable_v<swappable_not_assignable>);
static_assert(noexcept(
    std::swap(std::declval<std::optional<swappable_not_assignable>&>(),
              std::declval<std::optional<swappable_not_assignable>&>())));
static_assert(
    std::is_same_v<decltype(std::optional<int>{} <=> std::optional<long>{}),
                   std::strong_ordering>);
static_assert(std::is_same_v<decltype(std::optional<int>{} <=> std::nullopt),
                             std::strong_ordering>);
static_assert(std::is_same_v<decltype(std::nullopt <=> std::optional<int>{}),
                             std::strong_ordering>);
static_assert(std::is_same_v<decltype(std::optional<int>{} <=> 1L),
                             std::strong_ordering>);
static_assert(std::is_same_v<decltype(1L <=> std::optional<int>{}),
                             std::strong_ordering>);

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
  psyassert(a && a->value == 0);

  a.emplace(8);
  b = static_cast<decltype(a)&&>(a);
  psyassert(b->value == 8);
  psyassert(a && a->value == 0);

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
  psyassert(!(low < std::nullopt));
  psyassert(low > std::nullopt);
  psyassert(!(low <= std::nullopt));
  psyassert(low >= std::nullopt);
  psyassert(std::nullopt < low);
  psyassert(!(std::nullopt > low));
  psyassert(std::nullopt <= low);
  psyassert(!(std::nullopt >= low));
  psyassert(empty <= std::nullopt);
  psyassert(std::nullopt >= empty);

  const std::optional<comparison_probe> comparison_low{{1}};
  const std::optional<comparison_probe> comparison_high{{2}};
  (void)(comparison_low == comparison_high);
  psyassert(comparison_called == '=');
  (void)(comparison_low != comparison_high);
  psyassert(comparison_called == '!');
  (void)(comparison_low < comparison_high);
  psyassert(comparison_called == '<');
  (void)(comparison_low > comparison_high);
  psyassert(comparison_called == '>');
  (void)(comparison_low <= comparison_high);
  psyassert(comparison_called == 'l');
  (void)(comparison_low >= comparison_high);
  psyassert(comparison_called == 'g');

  const comparison_probe comparison_value{2};
  (void)(comparison_low == comparison_value);
  psyassert(comparison_called == '=');
  (void)(comparison_value == comparison_low);
  psyassert(comparison_called == '=');
  (void)(comparison_low != comparison_value);
  psyassert(comparison_called == '!');
  (void)(comparison_value != comparison_low);
  psyassert(comparison_called == '!');
  (void)(comparison_low < comparison_value);
  psyassert(comparison_called == '<');
  (void)(comparison_value < comparison_low);
  psyassert(comparison_called == '<');
  (void)(comparison_low > comparison_value);
  psyassert(comparison_called == '>');
  (void)(comparison_value > comparison_low);
  psyassert(comparison_called == '>');
  (void)(comparison_low <= comparison_value);
  psyassert(comparison_called == 'l');
  (void)(comparison_value <= comparison_low);
  psyassert(comparison_called == 'l');
  (void)(comparison_low >= comparison_value);
  psyassert(comparison_called == 'g');
  (void)(comparison_value >= comparison_low);
  psyassert(comparison_called == 'g');

  const std::optional<spaceship_probe> spaceship_low{{1}};
  const std::optional<spaceship_probe> spaceship_high{{2}};
  const spaceship_probe spaceship_value{2};
  psyassert((spaceship_low <=> spaceship_high) < 0);
  psyassert(comparison_called == '3');
  psyassert((spaceship_low <=> spaceship_value) < 0);
  psyassert(comparison_called == '3');
  psyassert((spaceship_value <=> spaceship_low) > 0);
  psyassert(comparison_called == '3');

  psyassert((empty <=> std::optional<int>{}) == 0);
  psyassert((empty <=> low) < 0);
  psyassert((low <=> empty) > 0);
  psyassert((low <=> high) < 0);
  psyassert((empty <=> std::nullopt) == 0);
  psyassert((low <=> std::nullopt) > 0);
  psyassert((std::nullopt <=> low) < 0);
  psyassert((empty <=> 0) < 0);
  psyassert((low <=> 1) == 0);
  psyassert((2 <=> low) > 0);

  std::optional<move_only> swap_left(std::in_place, 10);
  std::optional<move_only> swap_right(std::in_place, 20);
  swap_left.swap(swap_right);
  psyassert(swap_left->value == 20);
  psyassert(swap_right->value == 10);
  swap_right.reset();
  swap_left.swap(swap_right);
  psyassert(!swap_left);
  psyassert(swap_right->value == 20);

  std::optional<swappable_not_assignable> nonassignable_left(std::in_place, 1);
  std::optional<swappable_not_assignable> nonassignable_right(std::in_place, 2);
  std::swap(nonassignable_left, nonassignable_right);
  psyassert(nonassignable_left->value == 2);
  psyassert(nonassignable_right->value == 1);
  nonassignable_right.reset();
  std::swap(nonassignable_left, nonassignable_right);
  psyassert(!nonassignable_left);
  psyassert(nonassignable_right->value == 2);

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
