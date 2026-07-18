#include "psyassert.h"
#include <type_traits>

int main() {
  static_assert(std::is_same_v<int, int>);
  static_assert(std::negation_v<std::false_type>);
  static_assert(!std::negation_v<std::true_type>);
}
