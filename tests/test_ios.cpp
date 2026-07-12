#include <cassert>
#include <ios>

int main() {
  assert(std::ios_base::goodbit == 0);
  std::ios ios(nullptr);
  assert(!ios.rdbuf());
}
