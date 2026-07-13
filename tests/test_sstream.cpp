#include "psyassert.h"
#include <sstream>

int main() {
  std::ostringstream os;
  os << 42;
  psyassert(os.str() == "42");

  std::ostringstream overwrite("abcd");
  overwrite << 12;
  psyassert(overwrite.str() == "12cd");
  overwrite.str("wxyz");
  overwrite << 'q';
  psyassert(overwrite.str() == "qxyz");
}
