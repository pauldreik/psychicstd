#include "psyassert.h"
#include <limits>
#include <sstream>
#include <streambuf>
#include <string>

class seekbuf : public std::streambuf {
public:
  int calls = 0;

protected:
  pos_type seekoff(off_type off, std::ios_base::seekdir,
                   std::ios_base::openmode which) override {
    psyassert(which == std::ios_base::in);
    ++calls;
    return pos_type(off);
  }
};

int main() {
  std::istream null_stream(nullptr);
  psyassert(null_stream.fail());
  int null_value = 0;
  null_stream >> null_value;
  psyassert(null_stream.fail());

  std::istringstream in("42");
  int x = 0;
  in >> x;
  psyassert(x == 42);

  // sentry's automatic whitespace-skip must recognize tab/newline/CR, not
  // just plain space -- real trigger: tzdata's leapseconds file is
  // tab-separated ("Leap\t1972\tJun\t30\t..."), parsed with
  // exceptions(failbit|badbit) set, so a stall here throws ios_base::failure
  // rather than silently mis-parsing.
  std::istringstream tab_fields("1\t2\n3\r4");
  int field1 = 0, field2 = 0, field3 = 0, field4 = 0;
  tab_fields >> field1 >> field2 >> field3 >> field4;
  psyassert(field1 == 1 && field2 == 2 && field3 == 3 && field4 == 4);
  psyassert(!tab_fields.fail());

  std::istringstream leading_tab("\t\n\r  99");
  int leading_value = 0;
  leading_tab >> leading_value;
  psyassert(leading_value == 99);
  psyassert(!leading_tab.fail());

  std::string temporary_value;
  std::istringstream temporary("temporary");
  temporary >> temporary_value;
  psyassert(temporary_value == "temporary");
  psyassert(temporary.eof());

  std::istringstream hex_input("a");
  hex_input >> std::hex >> x;
  psyassert(x == 10);

  std::istringstream invalid_float("invalid");
  double d = 0;
  invalid_float >> d;
  psyassert(invalid_float.fail());

  std::istringstream long_double_input("1.25");
  long double ld = 0;
  long_double_input >> ld;
  psyassert(ld == 1.25L);

#if defined(PSYCHICSTD_TEST_PSYCHICSTD)
  // strtod may report ERANGE for a valid nonzero subnormal.
  std::istringstream smallest_float("4.9406564584124654e-324");
  smallest_float >> d;
  psyassert(!smallest_float.fail());
  psyassert(d > 0);
#endif

  std::istringstream overflow("-1234567890123456");
  overflow >> x;
  psyassert(x == std::numeric_limits<int>::min());
  psyassert(overflow.fail());

  std::istringstream unsigned_overflow("65536");
  unsigned short us = 0;
  unsigned_overflow >> us;
  psyassert(us == std::numeric_limits<unsigned short>::max());
  psyassert(unsigned_overflow.fail());

  std::istringstream ull_overflow("18446744073709551616");
  unsigned long long ull = 0;
  ull_overflow >> ull;
  psyassert(ull == std::numeric_limits<unsigned long long>::max());
  psyassert(ull_overflow.fail());

  std::istringstream chars("ab");
  psyassert(chars.get() == 'a');

  std::istringstream line("ab*");
  char text[3];
  line.getline(text, 3, '*');
  psyassert(line.gcount() == 3);
  chars.putback('a');
  psyassert(chars.get() == 'a');

  std::stringbuf empty;
  std::istream throwing(&empty);
  throwing.exceptions(std::ios_base::eofbit);
  bool threw = false;
  try {
    throwing.get();
  } catch (const std::ios_base::failure&) {
    threw = true;
  }
  psyassert(threw);
  psyassert(throwing.eof());

  std::istringstream whitespace("  ");
  std::ws(whitespace);
  psyassert(whitespace.eof());
  psyassert(!whitespace.fail());
  std::ws(whitespace);
  psyassert(whitespace.fail());

  std::stringbuf peek_empty;
  std::istream peek_throwing(&peek_empty);
  peek_throwing.exceptions(std::ios_base::eofbit);
  threw = false;
  try {
    (void)peek_throwing.peek();
  } catch (const std::ios_base::failure&) {
    threw = true;
  }
  psyassert(threw);
  psyassert(peek_throwing.eof());
  psyassert(!peek_throwing.fail());

  std::stringbuf advancing("ab");
  psyassert(advancing.sgetc() == 'a');
  psyassert(advancing.snextc() == 'b');
  psyassert(advancing.snextc() == std::char_traits<char>::eof());

  seekbuf seeking_buffer;
  std::istream seeking(&seeking_buffer);
  seeking.seekg(5, std::ios_base::cur);
  psyassert(seeking.good());
  psyassert(seeking_buffer.calls == 1);
  seeking.seekg(-1, std::ios_base::beg);
  psyassert(seeking.fail());
  psyassert(seeking_buffer.calls == 2);

  seekbuf position_buffer;
  std::istream position_seeking(&position_buffer);
  position_seeking.seekg(std::streampos(1));
  psyassert(position_seeking.fail());

  seekbuf eof_seeking_buffer;
  std::istream eof_seeking(&eof_seeking_buffer);
  eof_seeking.setstate(std::ios_base::eofbit);
  eof_seeking.seekg(5, std::ios_base::beg);
  psyassert(eof_seeking.good());
  psyassert(eof_seeking_buffer.calls == 1);
}
