// Reverse include order of test_cstdlib.cpp: a direct #include <stdlib.h>
// first, then <cstdlib> -- its declarations must merge with the libc ones.
#include <stdlib.h>

#include <cstdlib>

#include "psyassert.h"

static_assert(sizeof(std::size_t) == sizeof(::size_t));

int main() {
  psyassert(std::atoi("-7") == -7);
  char* end = nullptr;
  psyassert(std::strtod("4.5", &end) == 4.5);
  std::div_t d = std::div(9, 4);
  psyassert(d.quot == 2 && d.rem == 1);
  void* p = std::malloc(16);
  psyassert(p != nullptr);
  std::free(p);
  return 0;
}
