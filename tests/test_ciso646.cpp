#include <cassert>
#include <ciso646>

// <ciso646> historically defined and/or/not etc. as macros for compilers
// without digraph support; in C++ these are keywords already, so this header
// is an empty stub. Exercise the alternative tokens it used to provide.
int main() {
  bool a = true, b = false;
  assert(a and not b);
  assert(a or b);
  assert((a bitand true) == a);
  assert((a bitor b) == a);
  assert((a xor b) == true);
  int mask = 0b1010;
  mask and_eq 0b1110;
  assert(mask == 0b1010);
}
