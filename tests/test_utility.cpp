#include <cassert>
#include <utility>

int main() {
  std::pair<int, double> p{42, 3.14};
  assert(p.first == 42);
}
