#include "psyassert.h"
#include <ctime>
#include <locale>

class passthrough_codecvt : public std::codecvt<char, char, std::mbstate_t> {
public:
  bool called = false;
  ~passthrough_codecvt() override = default;

protected:
  bool do_always_noconv() const noexcept override { return true; }
  result do_out(std::mbstate_t&, const char* from, const char*,
                const char*& from_next, char* to, char*,
                char*& to_next) const override {
    const_cast<passthrough_codecvt*>(this)->called = true;
    from_next = from;
    to_next = to;
    return noconv;
  }
  result do_unshift(std::mbstate_t&, char* to, char*,
                    char*& to_next) const override {
    to_next = to;
    return noconv;
  }
  int do_max_length() const noexcept override { return 1; }
  int do_encoding() const noexcept override { return 1; }
};
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

  passthrough_codecvt passthrough;
  const char* out_next = nullptr;
  char converted[3]{};
  char* converted_next = nullptr;
  result = passthrough.out(state, input, input + 3, out_next, converted,
                           converted + 3, converted_next);
  psyassert(result == std::codecvt_base::noconv);
  psyassert(passthrough.called);
}
