#include <cassert>
#include <string>
#include <system_error>

// A tiny custom error category, the pattern libraries use to report their
// own error codes through the standard error_code/system_error machinery.
class parse_error_category : public std::error_category {
public:
  const char* name() const noexcept override { return "parse"; }
  std::string message(int ev) const override {
    switch (ev) {
    case 1:
      return "unexpected token";
    case 2:
      return "unterminated string";
    default:
      return "unknown parse error";
    }
  }
};

const std::error_category& parse_category() {
  static parse_error_category c;
  return c;
}

std::error_code make_error_code(int ev) { return {ev, parse_category()}; }

int main() {
  std::error_code ec = make_error_code(1);
  assert(ec.message() == "unexpected token");
  assert(static_cast<bool>(ec));

  std::error_code ok(0, parse_category());
  assert(!static_cast<bool>(ok));

  bool threw = false;
  try {
    throw std::system_error(ec, "parsing failed");
  } catch (const std::system_error& e) {
    threw = true;
    assert(e.code() == ec);
    std::string what = e.what();
    assert(what.find("parsing failed") != std::string::npos);
  }
  assert(threw);
}
