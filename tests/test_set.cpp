#include <cassert>
#include <set>

int main() {
  std::set<int> s;
  s.insert(42);
  assert(s.count(42) == 1);
}
