#include <version>

// <version> only defines feature-test macros; a compile-time check is the
// whole test. Guard each with #ifdef since psychicstd only defines a subset
// of what libstdc++ advertises -- the point is that where a macro exists,
// its value is sane and SD-6 compliant (a plausible YYYYMML date).
#ifndef __cpp_lib_span
#error "__cpp_lib_span should be defined"
#endif
static_assert(__cpp_lib_span >= 201803L);

#ifndef __cpp_lib_bit_cast
#error "__cpp_lib_bit_cast should be defined"
#endif
static_assert(__cpp_lib_bit_cast >= 201806L);

#ifndef __cpp_lib_three_way_comparison
#error "__cpp_lib_three_way_comparison should be defined"
#endif
static_assert(__cpp_lib_three_way_comparison >= 201711L);

#ifndef __cpp_lib_byte
#error "__cpp_lib_byte should be defined"
#endif
static_assert(__cpp_lib_byte >= 201603L);

int main() {}
