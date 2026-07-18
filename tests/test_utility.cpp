#include "psyassert.h"
#include <type_traits>
#include <utility>

enum class flavor : unsigned short { plain = 7 };

struct move_prefers_copy {
  move_prefers_copy() = default;
  move_prefers_copy(const move_prefers_copy&) = default;
  move_prefers_copy(move_prefers_copy&&) noexcept(false) {}
};

int main() {
  std::pair<int, double> p{42, 3.14};
  psyassert(p.first == 42);

  int x = 5;
  static_assert(std::is_same_v<decltype(std::as_const(x)), const int&>);
  psyassert(&std::as_const(x) == &x);

  static_assert(std::is_same_v<decltype(std::move_if_noexcept(x)), int&&>);
  move_prefers_copy m;
  static_assert(
      std::is_same_v<decltype(std::move_if_noexcept(m)), const move_prefers_copy&>);

  static_assert(std::is_same_v<decltype(std::to_underlying(flavor::plain)),
                               unsigned short>);
  static_assert(std::to_underlying(flavor::plain) == 7);
}
