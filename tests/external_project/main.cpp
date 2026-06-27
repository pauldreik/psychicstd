#include <cassert>
#include <numeric>

int main() {
  assert(std::gcd(12, 8) == 4);
  assert(std::gcd(0, 5) == 5);
}
