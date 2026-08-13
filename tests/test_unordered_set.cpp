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

static void test_erase_if() {
  std::unordered_set<int> set{1, 2, 3, 4, 5};
  psyassert(std::erase_if(set, [](int value) { return value % 2 == 0; }) == 2);
  psyassert(set.size() == 3);
  psyassert(set.contains(1) && set.contains(3) && set.contains(5));
  psyassert(std::erase_if(set, [](int) { return false; }) == 0);
  psyassert(std::erase_if(set, [](int) { return true; }) == 3);
  psyassert(set.empty());

  std::unordered_multiset<int> multiset{1, 2, 2, 3, 2};
  psyassert(std::erase_if(multiset, [](int value) { return value == 2; }) == 3);
  psyassert(multiset.size() == 2);
  psyassert(multiset.count(1) == 1 && multiset.count(3) == 1);
}

int main() {
  test_erase_if();

  std::unordered_set<int> s;
  s.reserve(100);
  psyassert(s.bucket_count() >= 100);
  s.insert(42);
  psyassert(s.count(42) == 1);
  psyassert(s.contains(42));
  psyassert(!s.contains(41));

  std::unordered_multiset<int> a{1, 1, 2};
  std::unordered_multiset<int> b{2, 1, 1};
  psyassert(a == b);
  auto one = a.find(1);
  psyassert(one != a.end());
  a.erase(one);
  psyassert(a.count(1) == 1);
  a.insert(1);
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
