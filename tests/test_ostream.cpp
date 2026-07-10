#include <cassert>
#include <iomanip>
#include <ostream>
#include <sstream>

// A custom manipulator, the same shape as std::endl: a plain function that
// operator<< recognizes and calls.
std::ostream& tab(std::ostream& os) { return os.put('\t'); }

int main() {
  std::ostringstream os;
  os << "a" << tab << "b" << std::endl;
  assert(os.str() == "a\tb\n");

  std::ostringstream padded;
  padded << std::setw(6) << std::setfill('*') << 42;
  assert(padded.str() == "****42");

  std::ostringstream left;
  left << std::left << std::setw(6) << std::setfill('.') << 7;
  assert(left.str() == "7.....");

  std::ostringstream hexed;
  hexed << std::hex << std::showbase << 255;
  assert(hexed.str() == "0xff");
}
