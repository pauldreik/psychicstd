#include "psyassert.h"
#include <type_traits>
#include <utility>

int main() {
  std::pair<int, double> p{42, 3.14};
  psyassert(p.first == 42);

  int x = 5;
  static_assert(std::is_same_v<decltype(std::as_const(x)), const int&>);
  psyassert(&std::as_const(x) == &x);
}
