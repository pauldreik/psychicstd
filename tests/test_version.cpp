#include <version>

static_assert(__cpp_lib_nonmember_container_access >= 201411L);
static_assert(__cpp_lib_uncaught_exceptions >= 201411L);
static_assert(__cpp_lib_logical_traits >= 201510L);
static_assert(__cpp_lib_byte >= 201603L);
static_assert(__cpp_lib_is_invocable >= 201703L);
static_assert(__cpp_lib_variant >= 201606L);

static_assert(__cpp_lib_char8_t >= 201907L);
static_assert(__cpp_lib_three_way_comparison >= 201907L);
static_assert(__cpp_lib_span >= 202002L);
static_assert(__cpp_lib_jthread >= 201911L);
static_assert(__cpp_lib_is_constant_evaluated >= 201811L);
static_assert(__cpp_lib_source_location >= 201907L);
static_assert(__cpp_lib_bit_cast >= 201806L);

int main() {}
