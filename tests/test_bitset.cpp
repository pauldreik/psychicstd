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

  const std::bitset<6> text_bits("101010");
  psyassert(text_bits == std::bitset<6>(42));
  psyassert(text_bits.to_ulong() == 42);
  psyassert(text_bits.to_ullong() == 42);
  psyassert(std::bitset<6>(0).none());
  psyassert(std::hash<std::bitset<6>>{}(text_bits) ==
            std::hash<std::bitset<6>>{}(std::bitset<6>(42)));

  std::bitset<633> large;
  large.set(632);
  psyassert(large.test(632));
  psyassert(large.count() == 1);
  large.flip();
  psyassert(large.count() == 632);
  large.reset();
  psyassert(large.none());
}
