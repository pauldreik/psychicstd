#include "psyassert.h"
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

struct rvalue_resize_operation {
  std::size_t operator()(char* data, std::size_t size) && {
    if (size)
      data[0] = 'x';
    return size != 0;
  }

  std::size_t operator()(char*, std::size_t) & = delete;
};

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
  allocated = std::string_view("view");
  psyassert(allocated == "view");
  const char range[] = "range";
  allocated = {range, range + 5};
  psyassert(allocated == "range");

  std::string s = "hello";
  psyassert(s.starts_with('h'));
  psyassert(s.starts_with("he"));
  psyassert(s.ends_with('o'));
  psyassert(s.ends_with("lo"));
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
  filled.resize_and_overwrite(6, [](char* data, size_t capacity) {
    psyassert(capacity == 6);
    data[0] = 'o';
    data[1] = 'k';
    return 2;
  });
  psyassert(filled == "ok");
  filled.resize_and_overwrite(1, rvalue_resize_operation{});
  psyassert(filled == "x");
  static_assert(__cpp_lib_string_resize_and_overwrite == 202110L);
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
  std::string empty_single_insert;
  pos = empty_single_insert.insert(empty_single_insert.begin(), '/');
  psyassert(pos == empty_single_insert.begin());
  psyassert(empty_single_insert == "/");
  std::string empty_fill_insert;
  pos = empty_fill_insert.insert(empty_fill_insert.cbegin(), 2, '!');
  psyassert(pos == empty_fill_insert.begin());
  psyassert(empty_fill_insert == "!!");
  std::string empty_zero_insert;
  pos = empty_zero_insert.insert(empty_zero_insert.cbegin(), 0, '?');
  psyassert(pos == empty_zero_insert.begin());
  psyassert(empty_zero_insert.empty());
  empty_zero_insert.insert(0, 0, '?');
  empty_zero_insert.insert(0, "", 0);
  psyassert(empty_zero_insert.empty());
  std::string range_insert;
  const std::vector<char> inserted{'a', 'b', 'c'};
  auto range_pos = range_insert.insert(range_insert.begin(), inserted.begin(),
                                       inserted.end());
  psyassert(range_pos == range_insert.begin());
  psyassert(range_insert == "abc");

  const std::vector<char> no_characters;
  std::string empty_range_insert;
  range_pos = empty_range_insert.insert(
      empty_range_insert.begin(), no_characters.begin(), no_characters.end());
  psyassert(range_pos == empty_range_insert.begin());
  psyassert(empty_range_insert.empty());

  range_pos = range_insert.insert(range_insert.begin() + 1,
                                  no_characters.begin(), no_characters.end());
  psyassert(range_pos == range_insert.begin() + 1);
  psyassert(range_insert == "abc");

  std::string self_insert = "abcd";
  range_pos = self_insert.insert(self_insert.begin() + 2, self_insert.begin(),
                                 self_insert.end());
  psyassert(range_pos == self_insert.begin() + 2);
  psyassert(self_insert == "ababcdcd");

  std::string overlapping_insert = "abcd";
  range_pos = overlapping_insert.insert(overlapping_insert.begin() + 1,
                                        overlapping_insert.begin() + 2,
                                        overlapping_insert.end());
  psyassert(range_pos == overlapping_insert.begin() + 1);
  psyassert(overlapping_insert == "acdbcd");

  std::istringstream input("xyz");
  std::string single_pass_insert = "ab";
  range_pos = single_pass_insert.insert(single_pass_insert.begin() + 1,
                                        std::istreambuf_iterator<char>(input),
                                        std::istreambuf_iterator<char>());
  psyassert(range_pos == single_pass_insert.begin() + 1);
  psyassert(single_pass_insert == "axyzb");
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
