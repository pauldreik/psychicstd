#include <cassert>
#include <numeric>

static_assert(std::gcd(12, 8) == 4);
static_assert(std::gcd(0, 5) == 5);
static_assert(std::gcd(7, 13) == 1);

int main() {
  assert(std::gcd(12, 8) == 4);
  assert(std::gcd(0, 5) == 5);
  assert(std::gcd(-12, 8) == 4);
  assert(std::gcd(7, 13) == 1);
  assert(std::gcd(0, 0) == 0);
}
