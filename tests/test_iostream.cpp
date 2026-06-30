#include <cassert>
#include <iostream>
#include <sstream>

int main() {
  std::ostringstream buf;
  auto* old = std::cout.rdbuf(buf.rdbuf());
  std::cout << "test";
  std::cout.rdbuf(old);
  assert(buf.str() == "test");
}
