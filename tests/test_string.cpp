#include "psyassert.h"
#include <string>

int main() {
#if !defined(_PSYCHICSTD_COMPATIBILITY_LEVEL) ||                               \
    _PSYCHICSTD_COMPATIBILITY_LEVEL >= 2
  psyassert(isspace(' '));
#endif

  std::string::allocator_type allocator;
  (void)allocator;

  std::string s = "hello";
  std::string source = "assign";
  psyassert(std::string(source, 1, 3) == "ssi");
  psyassert(std::string(source, 4, 99) == "gn");
  s.assign(source.begin() + 1, source.end() - 1);
  psyassert(s == "ssig");
  s.assign(source, 1, 3);
  psyassert(s == "ssi");

  s = "hello";
  psyassert(s.size() == 5);
  auto pos = s.insert(s.end() - 2, ':');
  psyassert(pos == s.end() - 3);
  psyassert(s == "hel:lo");
  s.insert(s.begin() + 1, 2, '!');
  psyassert(s == "h!!el:lo");
  s.insert(0, 2, '?');
  psyassert(s == "??h!!el:lo");
  s.insert(s.cbegin() + 2, 1, '#');
  psyassert(s == "??#h!!el:lo");
  s.replace(s.begin(), s.begin() + 3, std::string("ok"));
  psyassert(s == "okh!!el:lo");
  s.replace(0, 2, 3, '-');
  psyassert(s == "---h!!el:lo");

  // string <-> string_view in ?: must pick string_view (requires the
  // string_view ctor to be explicit per [string.cons]; hit by cmake).
  std::string_view sv = false ? s : std::string_view("vw");
  psyassert(sv == "vw");
}
