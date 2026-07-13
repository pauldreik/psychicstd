#include "psyassert.h"
#include <set>

int main() {
  std::set<int> s;
  s.insert(42);
  psyassert(s.count(42) == 1);
}
