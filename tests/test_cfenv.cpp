#include <cassert>
#include <cfenv>

#pragma STDC FENV_ACCESS ON

// Changing the rounding mode changes the result of an inexact computation:
// 1.0 / 3.0 rounds differently depending on FE_DOWNWARD vs FE_UPWARD.
int main() {
  std::fenv_t saved;
  std::fegetenv(&saved);

  // Operands must be volatile too, not just the result: with literal
  // operands the division is a constant expression, and without
  // -frounding-math the compiler is free to fold it at compile time with
  // the default rounding mode, making it deaf to fesetround() entirely.
  volatile double one = 1.0;
  volatile double three = 3.0;

  std::fesetround(FE_DOWNWARD);
  volatile double down = one / three;

  std::fesetround(FE_UPWARD);
  volatile double up = one / three;

  assert(down < up);

  std::fesetenv(&saved);

  std::feclearexcept(FE_ALL_EXCEPT);
  volatile double x = 1.0;
  volatile double zero = 0.0;
  volatile double inf = x / zero;
  (void)inf;
  assert(std::fetestexcept(FE_DIVBYZERO) != 0);
  std::feclearexcept(FE_ALL_EXCEPT);
}
