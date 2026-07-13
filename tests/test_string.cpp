#include "psyassert.h"
#include <string>

int main() {
  std::string s = "hello";
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
}
