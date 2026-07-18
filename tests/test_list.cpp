#include "psyassert.h"
#include <list>

#include "test_allocator.h"

int main() {
  std::list<int> l = {1, 2, 3};
  psyassert(l.front() == 1);

  std::allocator<int> a;
  const std::list<int> la(a);
  psyassert(la.get_allocator() == a);

  test_allocator<int> ta(1);
  const std::list<int, test_allocator<int>> tl(ta);
  psyassert(tl.get_allocator() == ta);
}
