#include <cassert>
#include <iosfwd>
#include <sstream>

// The point of <iosfwd> is to let a "header" declare stream-taking functions
// without pulling in the full <iostream>/<sstream> machinery. Mimic that
// separation here: this function is declared using only forward declarations.
std::ostream& greet(std::ostream& os, const char* name);

std::ostream& greet(std::ostream& os, const char* name) {
  os << "hello, " << name;
  return os;
}

int main() {
  std::ostringstream oss;
  greet(oss, "world");
  assert(oss.str() == "hello, world");

  static_assert(sizeof(std::streamsize) >= sizeof(long));
}
