#include <math.h>

#define CHECK_C_UNARY(function)                                                \
  static_assert(__is_same(decltype(::function##f(1.0F)), float));              \
  static_assert(__is_same(decltype(::function(1.0)), double));                 \
  static_assert(__is_same(decltype(::function##l(1.0L)), long double))

#define CHECK_C_BINARY(function)                                               \
  static_assert(__is_same(decltype(::function##f(1.0F, 2.0F)), float));        \
  static_assert(__is_same(decltype(::function(1.0, 2.0)), double));            \
  static_assert(__is_same(decltype(::function##l(1.0L, 2.0L)), long double))

CHECK_C_UNARY(sqrt);
CHECK_C_UNARY(exp);
CHECK_C_UNARY(log);
CHECK_C_UNARY(log10);
CHECK_C_UNARY(sin);
CHECK_C_UNARY(cos);
CHECK_C_UNARY(tan);
CHECK_C_UNARY(sinh);
CHECK_C_UNARY(cosh);
CHECK_C_UNARY(tanh);
CHECK_C_BINARY(atan2);
CHECK_C_BINARY(hypot);
CHECK_C_BINARY(pow);

int main() {}
