#include "psyassert.h"
#include <list>

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
}
