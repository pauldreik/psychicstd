#include "psyassert.h"
#include <type_traits>
#include <utility>

enum class flavor : unsigned short { plain = 7 };

struct move_prefers_copy {
  move_prefers_copy() = default;
  move_prefers_copy(const move_prefers_copy&) = default;
  move_prefers_copy(move_prefers_copy&&) noexcept(false) {}
};

struct move_only {
  int value;

  explicit move_only(int v) : value(v) {}
  move_only(const move_only&) = delete;
  move_only& operator=(const move_only&) = delete;
  move_only(move_only&& other) noexcept : value(other.value) {
    other.value = 0;
  }
  move_only& operator=(move_only&& other) noexcept {
    value = other.value;
    other.value = 0;
    return *this;
  }
};

int main() {
  std::pair<int, double> p{42, 3.14};
  psyassert(p.first == 42);
  psyassert(std::get<0>(p) == 42);
  static_assert(std::tuple_size<decltype(p)>::value == 2);
  static_assert(
      std::is_same_v<std::tuple_element<1, decltype(p)>::type, double>);
  std::pair<const int*, const double*> empty;
  psyassert(empty.first == nullptr);
  psyassert(empty.second == nullptr);

  int x = 5;
  static_assert(std::is_same_v<decltype(std::move(x)), int&&>);
  static_assert(std::is_same_v<decltype(std::forward<int&>(x)), int&>);
  static_assert(std::is_same_v<decltype(std::forward<int>(x)), int&&>);
  static_assert(std::is_same_v<decltype(std::as_const(x)), const int&>);
  psyassert(&std::as_const(x) == &x);

  static_assert(std::is_same_v<decltype(std::move_if_noexcept(x)), int&&>);
  move_prefers_copy m;
  static_assert(std::is_same_v<decltype(std::move_if_noexcept(m)),
                               const move_prefers_copy&>);

  static_assert(std::is_same_v<decltype(std::to_underlying(flavor::plain)),
                               unsigned short>);
  static_assert(std::to_underlying(flavor::plain) == 7);

  move_only first(1);
  move_only second(2);
  std::swap(first, second);
  psyassert(first.value == 2);
  psyassert(second.value == 1);

  move_only old = std::exchange(first, move_only(3));
  psyassert(old.value == 2);
  psyassert(first.value == 3);

  std::pair<move_only, move_only> moved_pair(move_only(4), move_only(5));
  psyassert(moved_pair.first.value == 4);
  psyassert(moved_pair.second.value == 5);

  auto made_pair = std::make_pair(move_only(6), move_only(7));
  psyassert(made_pair.first.value == 6);
  psyassert(made_pair.second.value == 7);
}
