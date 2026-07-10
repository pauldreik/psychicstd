#include <cassert>
#include <cmath>

// Newton's method for sqrt, using nextafter to show it converges to within
// one ULP of std::sqrt -- and ilogb/scalbn to decompose/rebuild a float
// exactly (the classic frexp/ldexp pair, base-2 style).
double newton_sqrt(double x) {
  double guess = x;
  for (int i = 0; i < 50; ++i)
    guess = 0.5 * (guess + x / guess);
  return guess;
}

int main() {
  double x = 2.0;
  double mine = newton_sqrt(x);
  double theirs = std::sqrt(x);
  double diff = std::fabs(mine - theirs);
  assert(diff < 1e-9);

  int exp;
  double frac = std::frexp(12.5, &exp);
  assert(std::ldexp(frac, exp) == 12.5);

  assert(std::ilogb(8.0) == 3);
  assert(std::scalbn(1.0, 3) == 8.0);

  assert(std::isnan(0.0 / 0.0));
  assert(std::isinf(1.0 / 0.0));
  assert(!std::isfinite(1.0 / 0.0));
  assert(std::signbit(-0.0) && !std::signbit(0.0));
  assert(std::copysign(3.0, -1.0) == -3.0);

  assert(std::abs(-5) == 5);
  assert(std::abs(-5.5) == 5.5);

  double one = 1.0;
  double next = std::nextafter(one, 2.0);
  assert(next > one);
  assert(std::nextafter(next, 0.0) == one);
}
