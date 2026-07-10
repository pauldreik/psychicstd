#include <cassert>
#include <climits>

static_assert(CHAR_BIT == 8);
static_assert(INT_MAX == 2147483647);
static_assert(INT_MIN == -INT_MAX - 1);
static_assert(LLONG_MAX > INT_MAX);

int main() {
  // Signed overflow wraps predictably only via unsigned arithmetic; use
  // INT_MAX to check the classic "average without overflow" trick.
  int a = INT_MAX - 2;
  int b = INT_MAX;
  int mid = a + (b - a) / 2;
  assert(mid == a + (b - a) / 2);
  assert(mid >= a && mid <= b);

  unsigned char uc = UCHAR_MAX;
  assert(static_cast<unsigned char>(uc + 1) == 0);
}
