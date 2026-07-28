#include "psyassert.h"
#include <list>

struct sortable_item {
  int key;
  int order = 0;

  bool operator<(const sortable_item& other) const { return key < other.key; }
};

template <typename T> struct stateful_allocator {
  using value_type = T;

  int state;

  explicit stateful_allocator(int state) : state(state) {}

  template <typename U>
  stateful_allocator(const stateful_allocator<U>& other) : state(other.state) {}

  T* allocate(std::size_t n) { return std::allocator<T>{}.allocate(n); }
  void deallocate(T* p, std::size_t n) { std::allocator<T>{}.deallocate(p, n); }

  template <typename U>
  bool operator==(const stateful_allocator<U>& other) const {
    return state == other.state;
  }
};

int main() {
  std::list<int> l = {1, 2, 3};
  psyassert(l.front() == 1);
  std::list<int>::const_iterator const_begin = l.begin();
  psyassert(l.begin() == const_begin);
  psyassert(const_begin == l.begin());

  std::allocator<int> a;
  const std::list<int> la(a);
  psyassert(la.get_allocator() == a);

  stateful_allocator<int> ta(1);
  const std::list<int, stateful_allocator<int>> tl(ta);
  psyassert(tl.get_allocator() == ta);

  std::list<int> source = {4, 5};
  l.splice(l.end(), source);
  psyassert(source.empty());
  psyassert(l.size() == 5);
  psyassert(l.back() == 5);

  auto reverse = l.rbegin();
  psyassert(*reverse++ == 5);
  psyassert(*reverse == 4);

  std::list<sortable_item> sortable = {{3}, {1}, {2}};
  sortable_item* first = &sortable.front();
  sortable_item* second = &*++sortable.begin();
  sortable_item* third = &*std::next(sortable.begin(), 2);
  sortable.sort();
  psyassert(&sortable.front() == second);
  psyassert(&*++sortable.begin() == third);
  psyassert(&sortable.back() == first);

  std::list<sortable_item> stable = {{2, 0}, {1, 1}, {2, 2}, {1, 3}};
  stable.sort();
  auto stable_it = stable.begin();
  psyassert(stable_it++->order == 1);
  psyassert(stable_it++->order == 3);
  psyassert(stable_it++->order == 0);
  psyassert(stable_it++->order == 2);

  stable.sort([](const sortable_item& a, const sortable_item& b) {
    return a.key > b.key;
  });
  psyassert(stable.front().key == 2);
  psyassert(stable.back().key == 1);

  std::list<int> many;
  for (int i = 0; i < 1024; ++i)
    many.push_back(i);
  int comparisons = 0;
  many.sort([&](int a, int b) {
    ++comparisons;
    return a < b;
  });
  psyassert(comparisons < 20 * 1024);

  many.clear();
  for (int i = 0; i < 1000; ++i)
    many.push_back((i * 37) % 1000);
  many.sort();
  int expected = 0;
  for (int item : many)
    psyassert(item == expected++);

  int throws_after = 10;
  bool threw = false;
  try {
    many.sort([&](int a, int b) {
      if (--throws_after == 0)
        throw 1;
      return a > b;
    });
  } catch (int) {
    threw = true;
  }
  psyassert(threw);
  std::size_t forward_count = 0;
  for (auto it = many.begin(); it != many.end(); ++it)
    ++forward_count;
  std::size_t reverse_count = 0;
  for (auto it = many.rbegin(); it != many.rend(); ++it)
    ++reverse_count;
  psyassert(forward_count == many.size());
  psyassert(reverse_count == many.size());
}
