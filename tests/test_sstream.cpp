#include <cassert>
#include <sstream>

int main() {
  std::ostringstream os;
  os << 42;
  assert(os.str() == "42");
}
