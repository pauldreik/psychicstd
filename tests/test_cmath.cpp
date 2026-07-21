#include "psyassert.h"
#include <cmath>
#include <type_traits>

// Catch2 imports std::nextafter into a scope where the C declaration is
// already visible. The double overload must be that same function.
using std::nextafter;
static_assert(std::is_same_v<decltype(nextafter(1.0, 2.0)), double>);

int main() {
  psyassert(std::fpclassify(0.0) == FP_ZERO);
  psyassert(std::fpclassify(1.0F) == FP_NORMAL);
  psyassert(std::fpclassify(HUGE_VAL) == FP_INFINITE);
  psyassert(std::fpclassify(0) == FP_ZERO);
  static_assert(std::is_same_v<decltype(std::sqrt(1.0F)), float>);
  static_assert(std::is_same_v<decltype(std::sqrt(1)), double>);
  static_assert(std::is_same_v<decltype(std::sqrt(1.0L)), long double>);
  static_assert(std::is_same_v<decltype(std::nextafter(1.0F, 2.0F)), float>);
  static_assert(std::is_same_v<decltype(std::nextafterf(1.0F, 2.0F)), float>);
  static_assert(
      std::is_same_v<decltype(std::nextafterl(1.0L, 2.0L)), long double>);
  psyassert(std::nextafter(500.0F, 499.0F) < 500.0F);
}
