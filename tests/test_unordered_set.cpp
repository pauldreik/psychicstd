#include "psyassert.h"
#include <unordered_set>

int main() {
  std::unordered_set<int> s;
  s.reserve(100);
  psyassert(s.bucket_count() >= 100);
  s.insert(42);
  psyassert(s.count(42) == 1);
}
