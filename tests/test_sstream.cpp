#include <cassert>
#include <sstream>

int main() {
  std::ostringstream os;
  os << 42;
  assert(os.str() == "42");

  std::ostringstream overwrite("abcd");
  overwrite << 12;
  assert(overwrite.str() == "12cd");
  overwrite.str("wxyz");
  overwrite << 'q';
  assert(overwrite.str() == "qxyz");
}
