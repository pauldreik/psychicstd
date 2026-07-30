#include "psyassert.h"
#include <sstream>
#include <utility>

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

struct TestStringbuf : std::stringbuf {
  using std::stringbuf::stringbuf;

  TestStringbuf(std::stringbuf&& other)
      : std::stringbuf(static_cast<std::stringbuf&&>(other)) {}
  TestStringbuf& operator=(TestStringbuf&& other) {
    std::stringbuf::operator=(static_cast<std::stringbuf&&>(other));
    return *this;
  }

  void bump_put(int n) { pbump(n); }
  int_type peek() { return underflow(); }
  int_type putback(int_type c) { return pbackfail(c); }
  char* put_base() const { return pbase(); }
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

  TestStringbuf bumped("123");
  bumped.bump_put(3);
  psyassert(bumped.snextc() == '2');
  psyassert(bumped.snextc() == '3');
  psyassert(bumped.snextc() == std::char_traits<char>::eof());
  bumped.sputc('4');
  psyassert(bumped.peek() == '4');

  TestStringbuf move_source("short");
  TestStringbuf moved(static_cast<std::stringbuf&&>(move_source));
  psyassert(moved.str() == "short");
  psyassert(move_source.str().empty());
  psyassert(!moved.put_base() || moved.put_base() != move_source.put_base());
  TestStringbuf move_assigned;
  move_assigned = static_cast<TestStringbuf&&>(moved);
  psyassert(move_assigned.str() == "short");
  psyassert(moved.str().empty());
  psyassert(!move_assigned.put_base() ||
            move_assigned.put_base() != moved.put_base());

  TestStringbuf output_putback("123");
  while (output_putback.snextc() != std::char_traits<char>::eof())
    ;
  psyassert(output_putback.putback('3') == '3');
  psyassert(output_putback.putback('3') == '3');
  psyassert(output_putback.str() == "133");
  TestStringbuf input_putback("123", std::ios::in);
  while (input_putback.snextc() != std::char_traits<char>::eof())
    ;
  psyassert(input_putback.putback('3') == '3');
  psyassert(input_putback.putback('3') == std::char_traits<char>::eof());
  psyassert(input_putback.str() == "123");

  std::stringbuf input_seek("0123", std::ios::in);
  psyassert(input_seek.pubseekoff(1, std::ios::beg, std::ios::out) == -1);
  psyassert(input_seek.pubseekpos(1, std::ios::out) == -1);
  std::stringbuf output_seek("0123", std::ios::out);
  psyassert(output_seek.pubseekoff(1, std::ios::beg, std::ios::in) == -1);
  psyassert(output_seek.pubseekpos(1, std::ios::in) == -1);
  std::stringbuf split_seek("0123");
  psyassert(split_seek.pubseekoff(1, std::ios::beg,
                                  std::ios::in | std::ios::out) == 1);
  psyassert(split_seek.sputc('x') == 'x');
  psyassert(split_seek.pubseekoff(0, std::ios::cur,
                                  std::ios::in | std::ios::out) == -1);

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
