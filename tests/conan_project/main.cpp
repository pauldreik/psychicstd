#include <fmt/format.h>
#include <locale>

struct punctuation : std::numpunct<char> {
protected:
  char do_decimal_point() const override { return ','; }
  char do_thousands_sep() const override { return '.'; }
  string_type do_grouping() const override { return "\3"; }
};

int main() {
  fmt::print("fmt + psychicstd: {}\n", 42);
  auto locale = std::locale(std::locale(), new punctuation);
  fmt::print("localized fmt: {}\n", fmt::format(locale, "{:L}", 1234567));
  return 0;
}
