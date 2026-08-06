#include "psyassert.h"
#include <bit>
#include <type_traits>

static_assert(std::is_integral_v<unsigned>);
static_assert(std::is_unsigned_v<unsigned>);
static_assert(std::is_same_v<std::remove_cv_t<const unsigned>, unsigned>);
static_assert(std::is_trivially_copyable_v<unsigned>);

int main() { psyassert(std::bit_width(8U) == 4); }
