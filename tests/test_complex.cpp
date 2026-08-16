#include "psyassert.h"
#include <complex>
#include <iomanip>
#include <limits>
#include <sstream>
#include <type_traits>

namespace {
struct unrelated_expression {};

template <typename T>
concept has_std_arg = requires(T value) { std::arg(value); };

template <typename T> void test_integral_output() {
  std::ostringstream stream;
  stream << std::complex<T>(12, 34);
  psyassert(stream.str() == "(12,34)");

  std::wostringstream wide_stream;
  wide_stream << std::complex<T>(12, 34);
  psyassert(wide_stream.str() == L"(12,34)");
}

static_assert(!has_std_arg<unrelated_expression>);
}

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
  static_assert(std::is_same_v<decltype(std::arg(1.0F)), float>);
  static_assert(std::is_same_v<decltype(std::arg(1)), double>);
  static_assert(std::is_same_v<decltype(std::arg(true)), double>);
  psyassert(std::arg(1.0F) == 0.0F);
  psyassert(std::arg(-1) > 3.0);

  std::complex<long double> wide(5.0L, 12.0L);
  psyassert(std::abs(wide) == 13.0L);
  const double maximum = std::numeric_limits<double>::max();
  psyassert(std::abs(std::complex<double>(maximum, maximum)) ==
            std::numeric_limits<double>::infinity());
  psyassert(std::abs(std::complex<double>(maximum, 1.0)) == maximum);
  std::complex<float> narrow(1.5F, -2.5F);
  std::complex<long double> converted = narrow;
  psyassert(converted.real() == 1.5L);
  psyassert(converted.imag() == -2.5L);
  narrow = converted;
  psyassert(narrow.real() == 1.5F);
  psyassert(narrow.imag() == -2.5F);

  c *= std::complex<double>(2.0, -1.0);
  psyassert(c == std::complex<double>(10.0, 5.0));
  psyassert(std::sqrt(std::complex<double>(-4.0, 0.0)).imag() == 2.0);
  psyassert(std::pow(std::complex<double>(0.0, 1.0), 2.0).real() < 0.0);
  const auto integer_power = std::pow(std::complex<float>(0.0F, 1.0F), 2);
  const auto real_error = integer_power.real() + 1.0;
  psyassert(real_error > -1e-12 && real_error < 1e-12);
  psyassert(integer_power.imag() > -1e-12 && integer_power.imag() < 1e-12);
#if defined(PSYCHICSTD_TEST_PSYCHICSTD)
  const auto negative_integer_power =
      std::pow(std::complex<float>(0.0F, 2.0F), -2);
  psyassert(negative_integer_power.real() == -0.25);
  psyassert(negative_integer_power.imag() == 0.0);
  const auto minimum_integer_power = std::pow(std::complex<float>(1.0F, 0.0F),
                                              std::numeric_limits<int>::min());
  psyassert(minimum_integer_power.real() == 1.0);
  psyassert(minimum_integer_power.imag() == 0.0);
#endif
  psyassert(std::sin(std::complex<double>(0.0, 0.0)) == std::complex<double>());
  const auto large_tangent = std::tan(std::complex<double>(0.0, 1000.0));
  psyassert(large_tangent.real() == 0.0);
  psyassert(large_tangent.imag() == 1.0);
  const auto large_hyperbolic_tangent =
      std::tanh(std::complex<double>(1000.0, 0.0));
  psyassert(large_hyperbolic_tangent.real() == 1.0);
  psyassert(large_hyperbolic_tangent.imag() == 0.0);
  const auto infinite_sine = std::sin(
      std::complex<double>(0.0, std::numeric_limits<double>::infinity()));
  psyassert(infinite_sine.real() == 0.0);
  psyassert(infinite_sine.imag() == std::numeric_limits<double>::infinity());
  const auto logarithm = std::log10(std::complex<double>(100.0, 0.0));
  psyassert(logarithm.real() > 1.99 && logarithm.real() < 2.01);
  psyassert(logarithm.imag() == 0.0);

  const auto infinite_root = std::sqrt(
      std::complex<double>(std::numeric_limits<double>::infinity(), 0.0));
  psyassert(infinite_root.real() == std::numeric_limits<double>::infinity());
  psyassert(infinite_root.imag() == 0.0);
  const auto infinite_exp = std::exp(
      std::complex<float>(std::numeric_limits<float>::infinity(), 0.0F));
  psyassert(infinite_exp.real() == std::numeric_limits<float>::infinity());
  psyassert(infinite_exp.imag() == 0.0F);

  std::ostringstream stream;
  stream << std::setw(9) << std::complex<double>(12.0, 34.0) << 'x';
  psyassert(stream.str() == "  (12,34)x");

  std::wostringstream wide_stream;
  wide_stream << std::setw(9) << std::complex<double>(12.0, 34.0);
  psyassert(wide_stream.str() == L"  (12,34)");

  test_integral_output<short>();
  test_integral_output<unsigned short>();
  test_integral_output<int>();
  test_integral_output<unsigned int>();
  test_integral_output<long>();
  test_integral_output<unsigned long>();
}
