#include "psyassert.h"
#include <sstream>

// User type with its own inserter; must work with lvalue and rvalue streams.
struct Loggable {};
std::ostream& operator<<(std::ostream& os, const Loggable&) {
  return os << "loggable";
}

// nlohmann-json-style type: implicitly converts to anything and provides
// deprecated stream operators. These must not be ambiguous with std::byte
// shift operators (regression: unconstrained operator<</>>(byte, I)).
struct JsonLike {
  int value = 0;
  template <class T> operator T() const { return T{}; }
  friend std::istream& operator<<(JsonLike& j, std::istream& is) {
    is >> j.value;
    return is;
  }
  friend std::ostream& operator>>(const JsonLike& j, std::ostream& os) {
    return os << j.value;
  }
};

int main() {
  std::ostringstream os;
  os << 42;
  psyassert(os.str() == "42");

  std::ostringstream overwrite("abcd");
  overwrite << 12;
  psyassert(overwrite.str() == "12cd");
  std::ostringstream at_end("abcd", std::ios::ate);
  at_end << 12;
  psyassert(at_end.str() == "abcd12");
  overwrite.str("wxyz");
  overwrite << 'q';
  psyassert(overwrite.str() == "qxyz");

  std::stringstream reposition("abcd");
  reposition.seekp(2);
  reposition << 'X';
  psyassert(reposition.str() == "abXd");

  std::stringstream input_only("value", std::ios::in);
  psyassert(input_only.get() == 'v');

  auto temporary = (std::ostringstream{} << std::string("temporary")).str();
  psyassert(temporary == "temporary");

  std::ostringstream user_lv;
  user_lv << Loggable{};
  psyassert(user_lv.str() == "loggable");
  auto user_rv = (std::ostringstream{} << Loggable{}).str();
  psyassert(user_rv == "loggable");

  std::stringstream jss("41");
  JsonLike j;
  j << jss;
  psyassert(j.value == 41);
  std::istringstream jis("42");
  j << jis;
  psyassert(j.value == 42);
  std::ostringstream jos;
  j >> jos;
  psyassert(jos.str() == "42");
}
