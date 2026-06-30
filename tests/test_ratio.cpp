#include <cassert>
#include <ratio>

int main() {
  static_assert(std::ratio<1, 1000>::num == 1);
  static_assert(std::ratio<1, 1000>::den == 1000);
}
