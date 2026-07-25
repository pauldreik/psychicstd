#include "psyassert.h"
#include <cstddef>
#include <unordered_set>

struct stateful_hash {
  int state;
  std::size_t operator()(int value) const { return value + state; }
};

struct stateful_equal {
  int state;
  bool operator()(int a, int b) const { return a + state == b + state; }
};

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

  std::unordered_set<int, stateful_hash, stateful_equal> original(
      123, stateful_hash{7}, stateful_equal{9});
  original.insert(42);
  psyassert(original.bucket_count() >= 123);

  auto copy = original;
  psyassert(copy.hash_function().state == 7);
  psyassert(copy.key_eq().state == 9);
  psyassert(copy.count(42) == 1);

  auto moved = static_cast<decltype(copy)&&>(copy);
  psyassert(moved.hash_function().state == 7);
  psyassert(moved.key_eq().state == 9);
  psyassert(moved.count(42) == 1);
}
