// Include order matters on Darwin, where <cstdlib> declares the libc names
// itself: a direct #include <stdlib.h> afterwards must merge cleanly.
#include <cstdlib>

#include <stdlib.h>

#include "psyassert.h"

static_assert(sizeof(std::size_t) == sizeof(void*));

extern "C" int test_cstdlib_cmp(const void* a, const void* b) {
  return *static_cast<const int*>(a) - *static_cast<const int*>(b);
}

int main() {
  psyassert(std::atoi("42") == 42);
  psyassert(std::atof("2.5") == 2.5);

  char* end = nullptr;
  psyassert(std::strtol("123", &end, 10) == 123);
  psyassert(std::strtoull("18446744073709551615", &end, 10) ==
            18446744073709551615ull);
  psyassert(std::strtod("1.5", &end) == 1.5);
  psyassert(std::strtof("0.25", &end) == 0.25f);

  int* p = static_cast<int*>(std::malloc(4 * sizeof(int)));
  psyassert(p != nullptr);
  p[0] = 3;
  p[1] = 1;
  p[2] = 2;
  p[3] = 0;
  std::qsort(p, 4, sizeof(int), test_cstdlib_cmp);
  psyassert(p[0] == 0 && p[1] == 1 && p[2] == 2 && p[3] == 3);
  int key = 2;
  void* found = std::bsearch(&key, p, 4, sizeof(int), test_cstdlib_cmp);
  psyassert(found != nullptr && *static_cast<int*>(found) == 2);
  std::free(p);

  std::div_t d = std::div(7, 2);
  psyassert(d.quot == 3 && d.rem == 1);
  std::ldiv_t ld = std::ldiv(-7L, 2L);
  psyassert(ld.quot == -3 && ld.rem == -1);
  std::lldiv_t lld = std::lldiv(7LL, -2LL);
  psyassert(lld.quot == -3 && lld.rem == 1);
  psyassert(std::abs(-5) == 5);

  // Names only <stdlib.h> provides (not part of std) must still work.
  psyassert(::setenv("PSYCHICSTD_TEST_VAR", "1", 1) == 0);
  psyassert(std::getenv("PSYCHICSTD_TEST_VAR") != nullptr);
  return 0;
}
