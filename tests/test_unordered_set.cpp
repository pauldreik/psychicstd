#include <cassert>
#include <unordered_set>

int main() {
  std::unordered_set<int> s;
  s.insert(42);
  assert(s.count(42) == 1);
}
