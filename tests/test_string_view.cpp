#include <cassert>
#include <string_view>

#define DUALASSERT(...)                                                        \
  do {                                                                         \
    assert(__VA_ARGS__);                                                       \
    static_assert(__VA_ARGS__);                                                \
  } while (0)

int main() {
  std::string_view sv = "hello";
  assert(sv.size() == 5);
  constexpr std::wstring_view wide = L"wide";
  static_assert(wide.size() == 4);
  assert(wide.size() == 4);

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
