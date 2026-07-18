#include "psyassert.h"
#include <stdexcept>
#include <string>

int main() {
#if defined(PSYCHICSTD_TEST_PSYCHICSTD) &&                                     \
    _PSYCHICSTD_COMPATIBILITY_LEVEL >= _PSYCHICSTD_COMPAT_DROPIN
  psyassert(isspace(' '));
  const char equal_left[] = "same";
  const char equal_right[] = "same";
  psyassert(std::equal(equal_left, equal_left + 4, equal_right));
  int value = 1;
  int&& moved = std::move(value);
  int&& forwarded = std::forward<int>(value);
  psyassert(&moved == &value);
  psyassert(&forwarded == &value);
#endif

  std::string::allocator_type allocator;
  (void)allocator;

  std::string s = "hello";
  psyassert(std::string().compare(0, 4, "test") < 0);
  psyassert(std::string("test").compare(0, 4, "test") == 0);
  psyassert(s.at(1) == 'e');
  const std::string const_s = s;
  psyassert(const_s.at(4) == 'o');
  bool at_threw = false;
  try {
    (void)s.at(s.size());
  } catch (const std::out_of_range&) {
    at_threw = true;
  }
  psyassert(at_threw);

  bool conversion_threw = false;
  try {
    (void)std::stoll("");
  } catch (const std::invalid_argument&) {
    conversion_threw = true;
  }
  psyassert(conversion_threw);
  conversion_threw = false;
  try {
    (void)std::stoi("c");
  } catch (const std::invalid_argument&) {
    conversion_threw = true;
  }
  psyassert(conversion_threw);
  conversion_threw = false;
  try {
    (void)std::stoull("file.cpp");
  } catch (const std::invalid_argument&) {
    conversion_threw = true;
  }
  psyassert(conversion_threw);

  std::string source = "assign";
  psyassert(std::string(source, 1, 3) == "ssi");
  psyassert(std::string(source, 4, 99) == "gn");
  s.assign(source.begin() + 1, source.end() - 1);
  psyassert(s == "ssig");
  s.assign(source, 1, 3);
  psyassert(s == "ssi");

  s = "hello";
  psyassert(s.size() == 5);
  s += s;
  psyassert(s == "hellohello");
  auto pos = s.insert(s.end() - 2, ':');
  psyassert(pos == s.end() - 3);
  psyassert(s == "hellohel:lo");
  s.insert(s.begin() + 1, 2, '!');
  psyassert(s == "h!!ellohel:lo");
  s.insert(0, 2, '?');
  psyassert(s == "??h!!ellohel:lo");
  s.insert(s.cbegin() + 2, 1, '#');
  psyassert(s == "??#h!!ellohel:lo");
  s.replace(s.begin(), s.begin() + 3, std::string("ok"));
  psyassert(s == "okh!!ellohel:lo");
  s.replace(0, 2, 3, '-');
  psyassert(s == "---h!!ellohel:lo");

  std::string erased = "abracadabra";
  psyassert(std::erase(erased, 'a') == 5);
  psyassert(erased == "brcdbr");
  psyassert(
      std::erase_if(erased, [](char c) { return c == 'b' || c == 'd'; }) == 3);
  psyassert(erased == "rcr");

  // string <-> string_view in ?: must pick string_view (requires the
  // string_view ctor to be explicit per [string.cons]; hit by cmake).
  std::string_view sv = false ? s : std::string_view("vw");
  psyassert(sv == "vw");
}
