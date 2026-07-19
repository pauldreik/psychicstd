#include "psyassert.h"
#include <map>
#include <tuple>

struct nonassignable {
  const int value;
  explicit nonassignable(int v) : value(v) {}
};

struct immovable {
  int value;
  explicit immovable(int v) : value(v) {}
  immovable(const immovable&) = delete;
  immovable(immovable&&) = delete;
  immovable& operator=(const immovable&) = delete;
  immovable& operator=(immovable&&) = delete;
};

struct recursive {
  std::map<recursive, recursive> children;
};

struct partially_ordered {
  int value;
  friend std::partial_ordering operator<=>(partially_ordered a,
                                           partially_ordered b) {
    if (a.value < 0 || b.value < 0)
      return std::partial_ordering::unordered;
    return a.value <=> b.value;
  }
  friend bool operator==(partially_ordered, partially_ordered) = default;
};

int main() {
  std::map<int, int> m;
  m[1] = 42;
  psyassert(m[1] == 42);
  auto hinted = m.emplace_hint(m.end(), 2, 24);
  psyassert(hinted->second == 24);

  std::map<int, nonassignable> immutable;
  immutable.emplace(2, 2);
  immutable.emplace(3, 3);
  immutable.emplace(4, 4);
  immutable.emplace(1, 1);
  psyassert(immutable.size() == 4);
  for (int i = 1; i <= 4; ++i)
    psyassert(immutable.at(i).value == i);

  auto inserted = immutable.emplace(std::piecewise_construct,
                                    std::make_tuple(5), std::make_tuple(5));
  psyassert(inserted.second);
  psyassert(inserted.first->second.value == 5);

  std::pair<const int, immovable> piecewise(
      std::piecewise_construct, std::make_tuple(1), std::make_tuple(11));
  psyassert(piecewise.first == 1);
  psyassert(piecewise.second.value == 11);

  std::map<int, immovable> node_map;
  auto third = node_map.emplace(std::piecewise_construct, std::make_tuple(3),
                                std::make_tuple(33));
  psyassert(third.second);
  psyassert(node_map.try_emplace(1, 11).second);
  psyassert(node_map.try_emplace(2, 22).second);
  psyassert(!node_map.try_emplace(2, 222).second);
  psyassert(node_map.size() == 3);
  auto current = node_map.begin();
  psyassert(current++->second.value == 11);
  psyassert(current++->second.value == 22);
  psyassert(current++->second.value == 33);
  psyassert(current == node_map.end());
  psyassert(node_map.rbegin()->second.value == 33);
  psyassert(node_map.erase(2) == 1);
  psyassert(!node_map.contains(2));
  static_assert(noexcept(node_map.swap(node_map)));

  std::map<int, partially_ordered> partial_a{{1, {-1}}};
  std::map<int, partially_ordered> partial_b{{1, {1}}};
  psyassert((partial_a <=> partial_b) == std::partial_ordering::unordered);

  std::multimap<int, int> multi;
  multi.emplace(1, 10);
  multi.emplace(1, 20);
  psyassert(multi.begin()->second == 10);
  psyassert(std::next(multi.begin())->second == 20);
}
