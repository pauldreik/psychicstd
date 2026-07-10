#include <cassert>
#include <cerrno>
#include <cstdlib>

int main() {
  errno = 0;
  double d = std::strtod("1e400", nullptr); // overflows double range
  assert(errno == ERANGE);
  assert(d > 0);

  errno = 0;
  long l = std::strtol("not a number", nullptr, 10);
  assert(l == 0);
  assert(errno == 0); // no conversion is not itself an error

  errno = E2BIG; // sanity-check a couple of the standard macros exist
  assert(errno == E2BIG);
  errno = 0;
  assert(errno == 0);
}
