#include <cassert>
#include <ranges>
#include <vector>

int main() {
  std::vector<int> v = {1, 2, 3};
  assert(*std::ranges::begin(v) == 1);
}
