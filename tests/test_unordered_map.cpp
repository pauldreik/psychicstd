#include "psyassert.h"
#include <unordered_map>

int main() {
  std::unordered_map<int, int> m;
  m[1] = 42;
  psyassert(m[1] == 42);
}
