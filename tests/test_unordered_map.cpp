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

  std::unordered_multimap<int, int> mm;
  mm.emplace(1, 10);
  mm.emplace(17, 99); // Same initial bucket, but not an equivalent key.
  mm.emplace(1, 20);
  auto range = mm.equal_range(1);
  int matches = 0;
  for (auto it = range.first; it != range.second; ++it) {
    psyassert(it->first == 1);
    ++matches;
  }
  psyassert(matches == 2);

  const auto& const_mm = mm;
  auto const_range = const_mm.equal_range(1);
  matches = 0;
  for (auto it = const_range.first; it != const_range.second; ++it)
    ++matches;
  psyassert(matches == 2);
}
