#include "psyassert.h"
#include <ratio>

int main() {
  static_assert(std::ratio<1, 1000>::num == 1);
  static_assert(std::ratio<1, 1000>::den == 1000);
  using one = std::ratio<1>;
  using two = std::ratio<2>;
  static_assert(std::ratio_less<one, two>::value);
  static_assert(std::ratio_less_equal<one, two>::value);
  static_assert(std::ratio_greater<two, one>::value);
  static_assert(std::ratio_greater_equal<two, one>::value);
}
