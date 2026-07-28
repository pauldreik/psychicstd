#include "psyassert.h"
#include <ios>
#include <type_traits>

int main() {
  static_assert(std::is_polymorphic<std::ios_base>::value);

  std::ios_base::Init init;
  (void)init;

  try {
    throw std::ios_base::failure("stream failed");
  } catch (const std::ios_base::failure& failure) {
    psyassert(failure.what()[0] == 's');
  }

  psyassert(std::ios_base::goodbit == 0);
  std::ios ios(nullptr);
  psyassert(!ios.rdbuf());
  psyassert(ios.bad());
  ios.exceptions(std::ios::eofbit);
  try {
    ios.exceptions(std::ios::badbit);
    psyassert(false);
  } catch (const std::ios::failure&) {
  }
  int first_word = std::ios_base::xalloc();
  int second_word = std::ios_base::xalloc();
  psyassert(first_word != second_word);
  psyassert(ios.iword(first_word) == 0);
  psyassert(ios.pword(second_word) == nullptr);
  ios.iword(first_word) = 42;
  ios.pword(second_word) = &ios;
  psyassert(ios.iword(first_word) == 42);
  psyassert(ios.pword(second_word) == &ios);
}
