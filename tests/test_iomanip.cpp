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

  std::string text = "a\\\"b";
  std::ostringstream quoted_out;
  quoted_out << std::quoted(text);
  psyassert(quoted_out.str() == "\"a\\\\\\\"b\"");
  std::istringstream quoted_in(quoted_out.str());
  std::string roundtrip;
  quoted_in >> std::quoted(roundtrip);
  psyassert(roundtrip == text);
}
