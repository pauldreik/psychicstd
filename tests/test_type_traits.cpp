#include "psyassert.h"
#include <type_traits>

using Fn = void();
using ConstFn = void() const;

int main() {
  static_assert(std::is_same_v<int, int>);
  static_assert(std::is_function_v<Fn>);
  static_assert(std::is_function_v<ConstFn>);
  static_assert(!std::is_function_v<int>);
  static_assert(std::negation_v<std::false_type>);
  static_assert(!std::negation_v<std::true_type>);
}
