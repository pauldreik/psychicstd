#include "psyassert.h"
#include <ctime>
#include <locale>
#include <sstream>

struct punct : std::numpunct<char> {
protected:
  char do_decimal_point() const override { return ','; }
  char do_thousands_sep() const override { return '.'; }
  string_type do_grouping() const override { return "\3"; }
};

int main() {
  std::locale classic;
  psyassert(std::has_facet<std::numpunct<char>>(classic));
  auto localized = std::locale(classic, new punct);

  psyassert(std::has_facet<std::numpunct<char>>(localized));
  const auto& facet = std::use_facet<std::numpunct<char>>(localized);
  psyassert(facet.decimal_point() == ',');
  psyassert(facet.thousands_sep() == '.');
  psyassert(facet.grouping() == "\3");

  // Facets inherited by a later locale must remain alive after its parent is
  // destroyed. This is how locale copies and chains are used in practice.
  auto copy = localized;
  localized = classic;
  psyassert(std::use_facet<std::numpunct<char>>(copy).decimal_point() == ',');

  std::tm time{};
  time.tm_year = 124;
  time.tm_mon = 0;
  time.tm_mday = 2;
  std::ostringstream out;
  std::ostreambuf_iterator<char> it(out);
  const char format[] = "%Y-%m-%d";
  std::use_facet<std::time_put<char>>(classic).put(it, out, ' ', &time, format,
                                                   format + sizeof(format) - 1);
  psyassert(out.str() == "2024-01-02");
  std::ostringstream year;
  std::use_facet<std::time_put<char>>(classic).put(
      std::ostreambuf_iterator<char>(year), year, ' ', &time, 'Y');
  psyassert(year.str() == "2024");

  std::mbstate_t state{};
  const char input[] = "abc";
  const char* from_next = nullptr;
  char32_t output[3]{};
  char32_t* to_next = nullptr;
  auto result =
      std::use_facet<std::codecvt<char32_t, char, std::mbstate_t>>(classic).in(
          state, input, input + 3, from_next, output, output + 3, to_next);
  psyassert(result == std::codecvt_base::ok);
  psyassert(from_next == input + 3 && to_next == output + 3);
  psyassert(output[0] == U'a' && output[2] == U'c');
}
