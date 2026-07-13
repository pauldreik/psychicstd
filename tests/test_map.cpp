#include "psyassert.h"
#include <map>

int main() {
  std::map<int, int> m;
  m[1] = 42;
  psyassert(m[1] == 42);
}
