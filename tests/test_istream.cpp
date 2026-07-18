#include "psyassert.h"
#include <limits>
#include <sstream>

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

  std::istringstream hex_input("a");
  hex_input >> std::hex >> x;
  psyassert(x == 10);

  std::istringstream overflow("-1234567890123456");
  overflow >> x;
  psyassert(x == std::numeric_limits<int>::min());
  psyassert(overflow.fail());

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
}
