#include <cassert>
#include <valarray>

int main() {
  std::valarray<int> v = {1, 2, 3};
  assert(v.size() == 3);
}
