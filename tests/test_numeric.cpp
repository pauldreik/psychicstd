#include "psyassert.h"
#include <numeric>

static_assert(std::gcd(12, 8) == 4);
static_assert(std::gcd(0, 5) == 5);
static_assert(std::gcd(7, 13) == 1);

int main() {
  psyassert(std::gcd(12, 8) == 4);
  psyassert(std::gcd(0, 5) == 5);
  psyassert(std::gcd(-12, 8) == 4);
  psyassert(std::gcd(7, 13) == 1);
  psyassert(std::gcd(0, 0) == 0);
}
