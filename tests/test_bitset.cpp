#include "psyassert.h"
#include <bitset>
#include <type_traits>

int main() {
  std::bitset<6> bits(42);
  psyassert(bits.size() == 6);
  psyassert(!bits[0] && bits[1] && !bits[2] && bits[3] && !bits[4] && bits[5]);
  bits[0] = true;
  bits[1].flip();
  psyassert(bits[0] && !bits[1]);
  static_assert(std::is_nothrow_assignable<decltype(bits[0]), bool>::value);

  std::bitset<633> large;
  large.set(632);
  psyassert(large.test(632));
  psyassert(large.count() == 1);
  large.flip();
  psyassert(large.count() == 632);
  large.reset();
  psyassert(large.none());
}
