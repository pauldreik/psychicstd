#include "psyassert.h"
#include <ios>
#include <sstream>
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
  std::ios null_ios(nullptr);
  null_ios.clear();
  psyassert(null_ios.bad());
  std::ostringstream tied_output;
  psyassert(ios.tie() == nullptr);
  psyassert(ios.tie(&tied_output) == nullptr);
  psyassert(ios.tie() == &tied_output);
  psyassert(ios.tie(nullptr) == &tied_output);
  auto initial_flags = null_ios.flags();
  auto old_flags =
      null_ios.setf(std::ios::right | std::ios::hex, std::ios::adjustfield);
  psyassert(old_flags == initial_flags);
  psyassert(null_ios.flags() ==
            ((initial_flags & ~std::ios::adjustfield) | std::ios::right));
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
