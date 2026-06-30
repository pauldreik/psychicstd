#include <cassert>
#include <sstream>

int main() {
  std::istringstream in("42");
  int x = 0;
  in >> x;
  assert(x == 42);
}
