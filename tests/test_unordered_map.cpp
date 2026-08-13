#include "psyassert.h"
#include <cstddef>
#include <unordered_map>

struct stateful_hash {
  int state;
  std::size_t operator()(int value) const { return value + state; }
};

struct stateful_equal {
  int state;
  bool operator()(int a, int b) const { return a + state == b + state; }
};

struct immovable {
  explicit immovable(int value) : value(value) {}
  immovable(const immovable&) = delete;
  immovable(immovable&&) = delete;
  int value;
};

static void test_erase_if() {
  std::unordered_map<int, int> map{{1, 1}, {2, 2}, {3, 3}, {4, 4}};
  psyassert(std::erase_if(map, [](const auto& value) {
              return value.first % 2 == 0;
            }) == 2);
  psyassert(map.size() == 2);
  psyassert(map.count(1) == 1 && map.count(3) == 1);
  psyassert(std::erase_if(map, [](const auto&) { return false; }) == 0);
  psyassert(std::erase_if(map, [](const auto&) { return true; }) == 2);
  psyassert(map.empty());

  std::unordered_multimap<int, int> multimap{{1, 1}, {2, 2}, {2, 3}, {3, 4}};
  psyassert(std::erase_if(multimap, [](const auto& value) {
              return value.first == 2;
            }) == 2);
  psyassert(multimap.size() == 2);
  psyassert(multimap.count(1) == 1 && multimap.count(3) == 1);
}

int main() {
  test_erase_if();

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
  std::unordered_map<int, immovable> immovable_map;
  psyassert(immovable_map.try_emplace(1, 42).first->second.value == 42);

  std::unordered_multimap<int, int> mm;
  mm.reserve(100);
  psyassert(mm.bucket_count() >= 100);
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

  auto erased_next = mm.erase(mm.find(17));
  psyassert(mm.count(17) == 0);
  psyassert(mm.size() == 2);
  psyassert(erased_next == mm.end() || erased_next->first == 1);

  std::unordered_multimap<int, int> equal_mm{{1, 20}, {17, 99}, {1, 10}};
  equal_mm.erase(equal_mm.find(17));
  psyassert(mm == equal_mm);
  equal_mm.emplace(1, 10);
  psyassert(!(mm == equal_mm));

  using stateful_map =
      std::unordered_map<int, int, stateful_hash, stateful_equal>;
  stateful_map stateful(4, stateful_hash{7}, stateful_equal{8});
  stateful.emplace(1, 2);
  stateful_map copied(stateful);
  psyassert(copied.hash_function().state == 7);
  psyassert(copied.key_eq().state == 8);
  stateful_map assigned;
  assigned = stateful;
  psyassert(assigned.hash_function().state == 7);
  stateful_map moved(static_cast<stateful_map&&>(copied));
  psyassert(moved.hash_function().state == 7);
}
