#include <cassert>
#include <cfloat>

static_assert(FLT_RADIX == 2);
static_assert(DBL_MANT_DIG > FLT_MANT_DIG);
static_assert(DBL_MAX_EXP > FLT_MAX_EXP);

int main() {
  // 1 + FLT_EPSILON must be the smallest float greater than 1; half of it
  // must round away to nothing when added to 1.
  volatile float one = 1.0f;
  assert(one + FLT_EPSILON != one);
  assert(one + FLT_EPSILON / 2.0f == one);

  assert(FLT_MIN > 0.0f);
  assert(DBL_MAX > 0.0);
}
