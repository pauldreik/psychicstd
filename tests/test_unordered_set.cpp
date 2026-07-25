#include "psyassert.h"
#include <unordered_set>

int main() {
  std::unordered_set<int> s;
  s.reserve(100);
  psyassert(s.bucket_count() >= 100);
  s.insert(42);
  psyassert(s.count(42) == 1);

  std::unordered_multiset<int> a{1, 1, 2};
  std::unordered_multiset<int> b{2, 1, 1};
  psyassert(a == b);
  b.insert(2);
  psyassert(!(a == b));
}
