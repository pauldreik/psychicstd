#include "psyassert.h"
#include <stdexcept>
#include <string>

int main() {
  std::string default_empty;
  default_empty.resize(0);
  psyassert(default_empty.empty());

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
  std::string allocated(allocator);
  allocated = "allocator";
  psyassert(allocated == "allocator");
  psyassert(allocated.get_allocator() == allocator);

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

  size_t conversion_pos = 0;
  psyassert(std::stoi("-42tail", &conversion_pos) == -42);
  psyassert(conversion_pos == 3);
  psyassert(std::stol("17") == 17L);
  psyassert(std::stoll("10000000000") == 10000000000LL);
  psyassert(std::stoul("42") == 42UL);
  psyassert(std::stoull("10000000000") == 10000000000ULL);
  psyassert(std::stof("1.5") == 1.5F);
  psyassert(std::stod("2.5") == 2.5);
  psyassert(std::stold("3.5") == 3.5L);

  psyassert(std::to_string(-42) == "-42");
  psyassert(std::to_string(42L) == "42");
  psyassert(std::to_string(42LL) == "42");
  psyassert(std::to_string(42U) == "42");
  psyassert(std::to_string(42UL) == "42");
  psyassert(std::to_string(42ULL) == "42");
  psyassert(std::to_string(1.5F) == "1.500000");
  psyassert(std::to_string(2.5) == "2.500000");
  psyassert(std::to_string(3.5L) == "3.500000");

  conversion_threw = false;
  try {
    (void)std::stoi("999999999999999999999999999999999");
  } catch (const std::out_of_range&) {
    conversion_threw = true;
  }
  psyassert(conversion_threw);
  conversion_threw = false;
  try {
    (void)std::stold("1e99999");
  } catch (const std::out_of_range&) {
    conversion_threw = true;
  }
  psyassert(conversion_threw);
  conversion_threw = false;
  try {
    (void)std::stol("x");
  } catch (const std::invalid_argument&) {
    conversion_threw = true;
  }
  psyassert(conversion_threw);
  conversion_threw = false;
  try {
    (void)std::stoul("x");
  } catch (const std::invalid_argument&) {
    conversion_threw = true;
  }
  psyassert(conversion_threw);
  conversion_threw = false;
  try {
    (void)std::stof("x");
  } catch (const std::invalid_argument&) {
    conversion_threw = true;
  }
  psyassert(conversion_threw);
  conversion_threw = false;
  try {
    (void)std::stod("x");
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
  char copied[8] = {};
  psyassert(source.copy(copied, 3, 1) == 3);
  psyassert(std::string(copied, 3) == "ssi");
  psyassert(source.copy(copied, 8, 4) == 2);
  bool copy_threw = false;
  try {
    (void)source.copy(copied, 1, source.size() + 1);
  } catch (const std::out_of_range&) {
    copy_threw = true;
  }
  psyassert(copy_threw);
  std::string filled(64, 'x');
  psyassert(filled.size() == 64 && filled.front() == 'x' &&
            filled.back() == 'x');
  filled.assign(32, 'a');
  filled.resize(48, 'b');
  filled.append(16, 'c');
  psyassert(filled ==
            std::string(32, 'a') + std::string(16, 'b') + std::string(16, 'c'));
  s.reserve(32);
  const char* storage = s.data();
  s.assign("replacement", 11);
  psyassert(s.data() == storage);
  psyassert(s == "replacement");

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
  s.replace(0, 3, "replace", 7);
  psyassert(s == "replaceh!!ellohel:lo");
  s.replace(0, 7, "ok");
  psyassert(s == "okh!!ellohel:lo");

  std::string erased = "abracadabra";
  psyassert(std::erase(erased, 'a') == 5);
  psyassert(erased == "brcdbr");
  psyassert(
      std::erase_if(erased, [](char c) { return c == 'b' || c == 'd'; }) == 3);
  psyassert(erased == "rcr");

  std::string empty;
  empty.erase(std::string::size_type{0});
  psyassert(empty.erase(empty.begin(), empty.end()) == empty.end());
  psyassert(empty.empty());

  std::string unchanged = "abc";
  psyassert(unchanged.erase(unchanged.begin() + 1, unchanged.begin() + 1) ==
            unchanged.begin() + 1);
  psyassert(unchanged == "abc");

  for (std::string::size_type i = 0; i < 1000; ++i)
    empty.append(i - empty.size(), '\0');
  psyassert(empty.size() == 999);

  // string <-> string_view in ?: must pick string_view (requires the
  // string_view ctor to be explicit per [string.cons]; hit by cmake).
  std::string_view sv = false ? s : std::string_view("vw");
  psyassert(sv == "vw");
}
