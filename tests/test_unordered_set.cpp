#include "psyassert.h"
#include <unordered_set>

int main() {
  std::unordered_set<int> s;
  s.insert(42);
  psyassert(s.count(42) == 1);
}
