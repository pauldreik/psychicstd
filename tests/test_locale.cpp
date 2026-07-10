#include <cassert>
#include <iomanip>
#include <locale>
#include <sstream>

// European-style numpunct: comma as decimal point, dot as thousands
// separator, grouped in 3s. Installing it via imbue() must actually change
// how the stream formats numbers.
struct euro_numpunct : std::numpunct<char> {
protected:
  char do_decimal_point() const override { return ','; }
  char do_thousands_sep() const override { return '.'; }
  std::string do_grouping() const override { return "\3"; }
};

int main() {
  std::ostringstream os;
  os.imbue(std::locale(std::locale::classic(), new euro_numpunct));
  os << std::fixed << std::setprecision(2) << 1234567.5;
  assert(os.str() == "1.234.567,50");

  std::locale a = std::locale::classic();
  std::locale b = a;
  assert(a == b);
}
