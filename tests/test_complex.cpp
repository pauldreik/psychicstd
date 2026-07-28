#include "psyassert.h"
#include <complex>
#include <type_traits>

int main() {
  static_assert(
      std::is_convertible_v<std::complex<float>, std::complex<double>>);
  static_assert(
      !std::is_convertible_v<std::complex<double>, std::complex<float>>);
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
  std::complex<float> narrow(1.5F, -2.5F);
  std::complex<long double> converted = narrow;
  psyassert(converted.real() == 1.5L);
  psyassert(converted.imag() == -2.5L);
  narrow = converted;
  psyassert(narrow.real() == 1.5F);
  psyassert(narrow.imag() == -2.5F);

  c *= std::complex<double>(2.0, -1.0);
  psyassert(c == std::complex<double>(10.0, 5.0));
}
