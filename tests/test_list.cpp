#include <cassert>
#include <list>

int main() {
  std::list<int> l = {1, 2, 3};
  assert(l.front() == 1);
}
