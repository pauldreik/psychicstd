#include "psyassert.h"
#include <complex>

int main() {
#if defined(PSYCHICSTD_TEST_PSYCHICSTD)
  constexpr std::complex<double> quotient =
      std::complex<double>(4.0, 2.0) / std::complex<double>(1.0, -1.0);
  static_assert(quotient == std::complex<double>(1.0, 3.0));
#endif

  std::complex<double> c(3.0, 4.0);
  psyassert(c.real() == 3.0);
  psyassert(std::abs(c) == 5.0);
  psyassert(std::arg(std::complex<double>(1.0, 0.0)) == 0.0);

  std::complex<long double> wide(5.0L, 12.0L);
  psyassert(std::abs(wide) == 13.0L);

  c *= std::complex<double>(2.0, -1.0);
  psyassert(c == std::complex<double>(10.0, 5.0));
}
