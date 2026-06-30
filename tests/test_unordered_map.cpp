#include <cassert>
#include <unordered_map>

int main() {
  std::unordered_map<int, int> m;
  m[1] = 42;
  assert(m[1] == 42);
}
