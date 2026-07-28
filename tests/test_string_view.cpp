#include "psyassert.h"
#include <stdexcept>
#include <string_view>

#define DUALASSERT(...)                                                        \
  do {                                                                         \
    psyassert(__VA_ARGS__);                                                    \
    static_assert(__VA_ARGS__);                                                \
  } while (0)

int main() {
  static_assert([] {
    char local[] = "view";
    return std::string_view(local) == "view";
  }());

  static constexpr char iterator_source[] = "view";
  constexpr std::string_view iterator_view(iterator_source,
                                           iterator_source + 4);
  static_assert(iterator_view == "view");
  static_assert(std::string_view(iterator_source, 0).empty());

  using namespace std::string_view_literals;
  static_assert("narrow"sv == std::string_view("narrow"));
  static_assert(L"wide"sv == std::wstring_view(L"wide"));
  static_assert(u8"utf8"sv == std::u8string_view(u8"utf8"));
  static_assert(u"utf16"sv == std::u16string_view(u"utf16"));
  static_assert(U"utf32"sv == std::u32string_view(U"utf32"));
  static_assert(L"ab"sv != L"ac"sv);
  static_assert(u8"ab"sv != u8"ac"sv);
  static_assert(u"ab"sv != u"ac"sv);
  static_assert(U"ab"sv != U"ac"sv);
  static_assert(L"wide"sv.starts_with(L"wid"sv));
  static_assert(u"utf16"sv.ends_with(u"16"sv));
  static_assert(U"ab"sv.compare(U"ac"sv) < 0);
#ifdef PSYCHICSTD_TEST_PSYCHICSTD
  static_assert(__cpp_lib_char8_t >= 201811L);
#endif

#if !defined(PSYCHICSTD_TEST_PSYCHICSTD) ||                                    \
    _PSYCHICSTD_COMPATIBILITY_LEVEL >= _PSYCHICSTD_COMPAT_DROPIN
  psyassert(std::min(2, 3) == 2);
#endif

  std::string_view sv = "hello";
  psyassert(sv.size() == 5);
  constexpr std::wstring_view wide = L"wide";
  static_assert(wide.size() == 4);
  psyassert(wide.size() == 4);
  psyassert(sv.compare(1, 3, "ell") == 0);
  psyassert(sv.compare(1, 3, "shell", 2, 3) == 0);
  psyassert(sv.compare(1, 3, "ellipsoid", 3) == 0);
  psyassert(sv.at(4) == 'o');
  psyassert(*sv.rbegin() == 'o');
  psyassert(sv.cbegin() == sv.begin());
  psyassert(sv.find("ellipsoid", 0, 3) == 1);
  psyassert(sv.find('o') == 4);
  static_assert(std::string_view("constexpr").find('p') == 7);
  psyassert(sv.rfind("hellish", std::string_view::npos, 4) == 0);
  psyassert(sv.find_first_of("xyzol", 0, 4) == 4);
  psyassert(sv.find_last_of("help", std::string_view::npos, 3) == 3);
  psyassert(sv.find_first_not_of("help", 0, 3) == 4);
  psyassert(sv.find_last_not_of("world", std::string_view::npos, 3) == 3);
  char copied[4] = {};
  psyassert(sv.copy(copied, 3, 1) == 3);
  psyassert(std::string_view(copied, 3) == "ell");

  bool threw = false;
  try {
    (void)sv.substr(sv.size() + 1);
  } catch (const std::out_of_range&) {
    threw = true;
  }
  psyassert(threw);

  threw = false;
  try {
    (void)sv.at(sv.size());
  } catch (const std::out_of_range&) {
    threw = true;
  }
  psyassert(threw);

  // find empty
  constexpr auto npos = std::string_view::npos;
  {
    constexpr std::string_view haystack = " ";
    constexpr std::string_view needle = "" /* non-null but zero size */;
    DUALASSERT(haystack.find(needle) == 0);
    DUALASSERT(haystack.find(needle, 1) == 1);
    DUALASSERT(haystack.compare(needle) != 0);
    DUALASSERT(needle.compare(haystack) != 0);
    DUALASSERT(haystack.starts_with(needle));
    DUALASSERT(haystack.ends_with(needle));
  }
  {
    constexpr std::string_view haystack = " ";
    constexpr std::string_view needle /*nullptr and zero size*/;
    DUALASSERT(haystack.find(needle) == 0);
    DUALASSERT(haystack.find(needle, 1) == 1);
    DUALASSERT(haystack.compare(needle) != 0);
    DUALASSERT(needle.compare(haystack) != 0);
    DUALASSERT(haystack.starts_with(needle));
    DUALASSERT(haystack.ends_with(needle));
  }
  {
    constexpr std::string_view haystack = "";
    constexpr std::string_view needle = "";
    DUALASSERT(haystack.find(needle) == 0);
    DUALASSERT(haystack.find(needle, 1) == npos);
    DUALASSERT(haystack.compare(needle) == 0);
    DUALASSERT(haystack == needle);
    DUALASSERT(haystack.starts_with(needle));
    DUALASSERT(haystack.ends_with(needle));
  }
  {
    constexpr std::string_view haystack /*nullptr and zero size*/;
    constexpr std::string_view needle /*nullptr and zero size*/;
    DUALASSERT(haystack.find(needle) == 0);
    DUALASSERT(haystack.find(needle, 1) == npos);
    DUALASSERT(haystack.compare(needle) == 0);
    DUALASSERT(haystack == needle);
    DUALASSERT(haystack.starts_with(needle));
    DUALASSERT(haystack.ends_with(needle));
  }
}
