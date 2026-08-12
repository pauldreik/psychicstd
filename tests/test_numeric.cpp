#include "psyassert.h"
#include <numeric>
#include <type_traits>

static_assert(std::gcd(12, 8) == 4);
static_assert(std::gcd(-12, 8) == 4);
static_assert(std::gcd(-12, -8) == 4);
static_assert(std::gcd(12, -8) == 4);
static_assert(std::gcd(0, 5) == 5);
static_assert(std::gcd(5, 0) == 5);
static_assert(std::gcd(0, 0) == 0);
static_assert(std::gcd(7, 13) == 1);

static_assert(std::is_same_v<decltype(std::gcd(2, 4)), int>);
static_assert(std::is_same_v<decltype(std::gcd(2U, 4)), unsigned int>);
static_assert(std::is_same_v<decltype(std::gcd(2, 4U)), unsigned int>);
#if !defined(PSYCHICSTD_TEST_PSYCHICSTD)
// psychic does not return the correct type when there is promotion. see
// https://eel.is/c++draft/numeric.ops#gcd
static_assert(std::is_same_v<decltype(std::gcd(char(2), char(4))), char>);
#endif
static_assert(std::is_same_v<decltype(std::gcd(char(2), int(4))), int>);
static_assert(std::is_same_v<decltype(std::gcd(int(2), char(4))), int>);

int main() {
  psyassert(std::gcd(12, 8) == 4);
  psyassert(std::gcd(0, 5) == 5);
  psyassert(std::gcd(-12, 8) == 4);
  psyassert(std::gcd(-12, -8) == 4);
  psyassert(std::gcd(12, -8) == 4);
  psyassert(std::gcd(7, 13) == 1);
  psyassert(std::gcd(0, 0) == 0);
}
