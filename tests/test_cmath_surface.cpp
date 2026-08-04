#include <cmath>

#define CHECK_UNARY_OVERLOADS(function)                                        \
  static_assert(__is_same(decltype(std::function(1.0F)), float));              \
  static_assert(__is_same(decltype(std::function(1.0)), double));              \
  static_assert(__is_same(decltype(std::function(1.0L)), long double));        \
  static_assert(__is_same(decltype(std::function(1)), double))

#define CHECK_BINARY_OVERLOADS(function)                                       \
  static_assert(__is_same(decltype(std::function(1.0F, 2.0F)), float));        \
  static_assert(__is_same(decltype(std::function(1.0, 2.0)), double));         \
  static_assert(__is_same(decltype(std::function(1.0L, 2.0L)), long double));  \
  static_assert(__is_same(decltype(std::function(1, 2)), double));             \
  static_assert(__is_same(decltype(std::function(1.0F, 2.0)), double));        \
  static_assert(__is_same(decltype(std::function(1.0, 2.0L)), long double))

CHECK_UNARY_OVERLOADS(sqrt);
CHECK_UNARY_OVERLOADS(exp);
CHECK_UNARY_OVERLOADS(log);
CHECK_UNARY_OVERLOADS(log10);
CHECK_UNARY_OVERLOADS(sin);
CHECK_UNARY_OVERLOADS(cos);
CHECK_UNARY_OVERLOADS(tan);
CHECK_UNARY_OVERLOADS(sinh);
CHECK_UNARY_OVERLOADS(cosh);
CHECK_UNARY_OVERLOADS(tanh);
CHECK_BINARY_OVERLOADS(atan2);
CHECK_BINARY_OVERLOADS(hypot);
CHECK_BINARY_OVERLOADS(pow);

enum class not_arithmetic {};

#define CHECK_REJECTS_ENUM_UNARY(function)                                     \
  template <typename T>                                                        \
  concept has_std_##function = requires(T value) { std::function(value); };    \
  static_assert(!has_std_##function<not_arithmetic>)

#define CHECK_REJECTS_ENUM_BINARY(function)                                    \
  template <typename T>                                                        \
  concept has_std_##function =                                                 \
      requires(T value) { std::function(value, value); };                      \
  static_assert(!has_std_##function<not_arithmetic>)

CHECK_REJECTS_ENUM_UNARY(sqrt);
CHECK_REJECTS_ENUM_UNARY(exp);
CHECK_REJECTS_ENUM_UNARY(log);
CHECK_REJECTS_ENUM_UNARY(log10);
CHECK_REJECTS_ENUM_UNARY(sin);
CHECK_REJECTS_ENUM_UNARY(cos);
CHECK_REJECTS_ENUM_UNARY(tan);
CHECK_REJECTS_ENUM_UNARY(sinh);
CHECK_REJECTS_ENUM_UNARY(cosh);
CHECK_REJECTS_ENUM_UNARY(tanh);
CHECK_REJECTS_ENUM_BINARY(atan2);
CHECK_REJECTS_ENUM_BINARY(hypot);
CHECK_REJECTS_ENUM_BINARY(pow);

#if defined(__GNUC__) && !defined(__clang__)
#define CHECK_CONSTEXPR_UNARY(function, input, expected)                       \
  static_assert(std::function(input##F) == expected##F);                       \
  static_assert(std::function(input) == expected);                             \
  static_assert(std::function(input##L) == expected##L);                       \
  static_assert(std::function(static_cast<int>(input)) == expected)

#define CHECK_CONSTEXPR_BINARY(function, left, right, expected)                \
  static_assert(std::function(left##F, right##F) == expected##F);              \
  static_assert(std::function(left, right) == expected);                       \
  static_assert(std::function(left##L, right##L) == expected##L);              \
  static_assert(std::function(static_cast<int>(left),                          \
                              static_cast<int>(right)) == expected)

CHECK_CONSTEXPR_UNARY(sqrt, 4.0, 2.0);
CHECK_CONSTEXPR_UNARY(exp, 0.0, 1.0);
CHECK_CONSTEXPR_UNARY(log, 1.0, 0.0);
CHECK_CONSTEXPR_UNARY(log10, 1.0, 0.0);
CHECK_CONSTEXPR_UNARY(sin, 0.0, 0.0);
CHECK_CONSTEXPR_UNARY(cos, 0.0, 1.0);
CHECK_CONSTEXPR_UNARY(tan, 0.0, 0.0);
CHECK_CONSTEXPR_UNARY(sinh, 0.0, 0.0);
CHECK_CONSTEXPR_UNARY(cosh, 0.0, 1.0);
CHECK_CONSTEXPR_UNARY(tanh, 0.0, 0.0);
CHECK_CONSTEXPR_BINARY(atan2, 0.0, 1.0, 0.0);
CHECK_CONSTEXPR_BINARY(hypot, 3.0, 4.0, 5.0);
CHECK_CONSTEXPR_BINARY(pow, 2.0, 3.0, 8.0);
#endif

int main() {}
