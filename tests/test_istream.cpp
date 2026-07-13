#include "psyassert.h"
#include <sstream>

int main() {
  std::istringstream in("42");
  int x = 0;
  in >> x;
  psyassert(x == 42);

  std::istringstream chars("ab");
  psyassert(chars.get() == 'a');
  chars.putback('a');
  psyassert(chars.get() == 'a');
}
