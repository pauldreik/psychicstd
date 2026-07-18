#include "psyassert.h"
#include <concepts>

using Function = void();
using ConstFunction = void() const;

int main() {
  static_assert(std::same_as<int, int>);
  static_assert(std::convertible_to<void, void>);
  static_assert(!std::convertible_to<ConstFunction, void>);
  static_assert(std::convertible_to<Function, Function&&>);
}
