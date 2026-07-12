#include <bitset>
#include <cassert>
#include <type_traits>

int main() {
  std::bitset<6> bits(42);
  assert(bits.size() == 6);
  assert(!bits[0] && bits[1] && !bits[2] && bits[3] && !bits[4] && bits[5]);
  bits[0] = true;
  bits[1].flip();
  assert(bits[0] && !bits[1]);
  static_assert(std::is_nothrow_assignable<decltype(bits[0]), bool>::value);
}
