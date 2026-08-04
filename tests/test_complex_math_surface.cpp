#include <complex>

template <typename T> constexpr void check_complex_math_surface() {
  using C = std::complex<T>;
  static_assert(__is_same(decltype(std::abs(C())), T));
  static_assert(__is_same(decltype(std::arg(C())), T));
  static_assert(__is_same(decltype(std::norm(C())), T));
  static_assert(__is_same(decltype(std::polar(T(), T())), C));
  static_assert(__is_same(decltype(std::sqrt(C())), C));
  static_assert(__is_same(decltype(std::exp(C())), C));
  static_assert(__is_same(decltype(std::log(C())), C));
  static_assert(__is_same(decltype(std::log10(C())), C));
  static_assert(__is_same(decltype(std::pow(C(), C())), C));
  static_assert(__is_same(decltype(std::pow(C(), T())), C));
  static_assert(__is_same(decltype(std::pow(T(), C())), C));
#if defined(PSYCHICSTD_TEST_PSYCHICSTD)
  using Promoted = std::complex<decltype(T() + double())>;
  static_assert(__is_same(decltype(std::pow(C(), 1)), Promoted));
  static_assert(__is_same(decltype(std::pow(C(), 1L)), Promoted));
  static_assert(__is_same(decltype(std::pow(C(), 1ULL)), Promoted));
#endif
  static_assert(__is_same(decltype(std::sin(C())), C));
  static_assert(__is_same(decltype(std::cos(C())), C));
  static_assert(__is_same(decltype(std::tan(C())), C));
  static_assert(__is_same(decltype(std::sinh(C())), C));
  static_assert(__is_same(decltype(std::cosh(C())), C));
  static_assert(__is_same(decltype(std::tanh(C())), C));
}

static_assert(__is_same(decltype(std::arg(1.0F)), float));
static_assert(__is_same(decltype(std::arg(1.0)), double));
static_assert(__is_same(decltype(std::arg(1.0L)), long double));
static_assert(__is_same(decltype(std::arg(1)), double));

int main() {
  check_complex_math_surface<float>();
  check_complex_math_surface<double>();
  check_complex_math_surface<long double>();
}
