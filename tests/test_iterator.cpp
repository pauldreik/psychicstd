#include <cassert>
#include <iterator>
#include <vector>

int main() {
  std::vector<int> v = {1, 2, 3};
  assert(*std::begin(v) == 1);
}
