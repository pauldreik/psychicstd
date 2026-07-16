// Regression test for <uchar.h>. Darwin's libc ships no <uchar.h>; consumers
// probe for it with __has_include (e.g. simdutf's C API header, which falls
// back to "#define char16_t uint16_t" and breaks the C++ keyword), so
// psychicstd must provide one that defines size_t and mbstate_t.
#include <uchar.h>

#include "psyassert.h"

#if !__has_include(<uchar.h>)
#error "<uchar.h> must be discoverable via __has_include"
#endif

static_assert(sizeof(char16_t) == 2);
static_assert(sizeof(char32_t) == 4);

int main() {
  mbstate_t st{};
  size_t n = sizeof st;
  psyassert(n > 0);

  const char16_t* s16 = u"psy";
  const char32_t* s32 = U"psy";
  psyassert(s16[0] == u'p');
  psyassert(s32[2] == U'y');
}
