#include <array>

int main() {
#if defined(_PSYCHICSTD_COMPATIBILITY_LEVEL) &&                                \
    _PSYCHICSTD_COMPATIBILITY_LEVEL >= _PSYCHICSTD_COMPAT_DROPIN
  int sum = 0;
  for (int value : {1, 2})
    sum += value;
  return sum == 3 ? 0 : 1;
#else
  return 0;
#endif
}
