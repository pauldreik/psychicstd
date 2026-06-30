#include <cassert>
#include <cstring>
#include <functional>

int main() {
  auto h = std::hash<const char*>{};
  assert(h("hello") != 0);
}
