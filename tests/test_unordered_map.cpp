#include "psyassert.h"
#include <unordered_map>

int main() {
  using map_type = std::unordered_map<int, int>;
  map_type::iterator empty_iterator;
  map_type::const_iterator empty_const_iterator;
  psyassert(empty_iterator == map_type::iterator{});
  psyassert(empty_const_iterator == map_type::const_iterator{});

  std::unordered_map<int, int> m;
  m.reserve(100);
  psyassert(m.bucket_count() >= 100);
  m[1] = 42;
  psyassert(m[1] == 42);
  psyassert(m.find(1) != m.cend());
}
