#include "psyassert.h"
#include <ratio>
#include <type_traits>

static_assert(std::is_same_v<std::ratio<1, 2>::type, std::ratio<1, 2>>);

int main() {
  static_assert(std::ratio<1, 1000>::num == 1);
  static_assert(std::ratio<1, 1000>::den == 1000);
  using one = std::ratio<1>;
  using two = std::ratio<2>;
  static_assert(std::ratio_less<one, two>::value);
  static_assert(std::ratio_less_equal<one, two>::value);
  static_assert(std::ratio_greater<two, one>::value);
  static_assert(std::ratio_greater_equal<two, one>::value);
  static_assert(std::ratio_equal_v<one, one>);
  static_assert(std::ratio_not_equal_v<one, two>);
  static_assert(std::ratio_less_v<one, two>);
  static_assert(std::ratio_less_equal_v<one, two>);
  static_assert(std::ratio_greater_v<two, one>);
  static_assert(std::ratio_greater_equal_v<two, one>);
}
