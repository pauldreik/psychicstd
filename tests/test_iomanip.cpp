#include <cassert>
#include <iomanip>
#include <sstream>

int main() {
  std::ostringstream os;
  os << std::setw(5) << 42;
  assert(os.str().size() >= 2);
}
