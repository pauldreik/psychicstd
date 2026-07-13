#include "psyassert.h"
#include <array>

int main() {
  std::array<int, 3> a = {1, 2, 3};
  psyassert(a.size() == 3);
}
