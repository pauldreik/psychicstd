#include <vector>

int main() {
#if defined(_PSYCHICSTD_COMPATIBILITY_LEVEL) &&                                \
    _PSYCHICSTD_COMPATIBILITY_LEVEL >= _PSYCHICSTD_COMPAT_DROPIN
  std::pair<int, int> value{1, 2};
  return value.first + value.second == 3 ? 0 : 1;
#else
  return 0;
#endif
}
