#include <cassert>
#include <random>

int main() {
  std::random_device rd;
  auto v = rd();
  assert(v >= rd.min());
}
