#include <cassert>
#include <optional>

int main() {
  std::optional<int> o = 42;
  assert(o.value() == 42);
}
