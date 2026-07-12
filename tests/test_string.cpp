#include <cassert>
#include <string>

int main() {
  std::string s = "hello";
  assert(s.size() == 5);
  auto pos = s.insert(s.end() - 2, ':');
  assert(pos == s.end() - 3);
  assert(s == "hel:lo");
  s.insert(s.begin() + 1, 2, '!');
  assert(s == "h!!el:lo");
  s.insert(0, 2, '?');
  assert(s == "??h!!el:lo");
  s.insert(s.cbegin() + 2, 1, '#');
  assert(s == "??#h!!el:lo");
}
