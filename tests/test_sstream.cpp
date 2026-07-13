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

  std::stringstream reposition("abcd");
  reposition.seekp(2);
  reposition << 'X';
  psyassert(reposition.str() == "abXd");

  std::stringstream input_only("value", std::ios::in);
  psyassert(input_only.get() == 'v');

  auto temporary = (std::ostringstream{} << std::string("temporary")).str();
  psyassert(temporary == "temporary");
}
