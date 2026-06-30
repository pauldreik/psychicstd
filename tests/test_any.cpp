#include <any>
#include <cassert>

int main() {
  std::any a = 42;
  assert(std::any_cast<int>(a) == 42);
}
