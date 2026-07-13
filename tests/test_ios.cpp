#include "psyassert.h"
#include <ios>

int main() {
  psyassert(std::ios_base::goodbit == 0);
  std::ios ios(nullptr);
  psyassert(!ios.rdbuf());
}
