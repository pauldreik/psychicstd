#include "psyassert.h"
#include <cmath>

int main() {
  psyassert(std::fpclassify(0.0) == FP_ZERO);
  psyassert(std::fpclassify(1.0F) == FP_NORMAL);
  psyassert(std::fpclassify(HUGE_VAL) == FP_INFINITE);
  psyassert(std::fpclassify(0) == FP_ZERO);
}
