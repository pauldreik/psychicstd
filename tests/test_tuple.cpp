#include <cassert>
#include <tuple>

int main() {
  auto t = std::make_tuple(1, 'a', 3.14);
  assert(std::get<0>(t) == 1);
}
