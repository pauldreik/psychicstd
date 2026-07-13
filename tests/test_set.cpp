#include "psyassert.h"
#include <set>

int main() {
  std::set<int, std::greater<int>> descending({1, 3, 2}, std::greater<int>{});
  psyassert(*descending.begin() == 3);

  std::set<int> s;
  s.insert(42);
  psyassert(s.count(42) == 1);

  std::multiset<int, std::greater<int>> multiple(std::greater<int>{});
  int values[] = {1, 3, 2, 3};
  multiple.insert(values, values + 4);
  psyassert(*multiple.begin() == 3);
  psyassert(multiple.count(3) == 2);
}
