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
}
