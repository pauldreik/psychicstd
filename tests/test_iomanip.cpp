#include "psyassert.h"
#include <ctime>
#include <iomanip>
#include <sstream>

int main() {
  std::ostringstream os;
  os << std::setw(5) << 42;
  psyassert(os.str().size() >= 2);

  std::tm value{};
  value.tm_year = 124;
  std::ostringstream out;
  out << std::put_time(&value, "%Y");
  psyassert(out.str() == "2024");
}
