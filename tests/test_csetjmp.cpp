#include "psyassert.h"
#include <csetjmp>

int main() {
  std::jmp_buf environment;
  int value = setjmp(environment);
  if (value == 0)
    std::longjmp(environment, 7);
  psyassert(value == 7);
}
