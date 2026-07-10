#include <cassert>
#include <clocale>
#include <cstring>

int main() {
  const char* prev = std::setlocale(LC_ALL, "C");
  assert(prev != nullptr);

  std::lconv* lc = std::localeconv();
  assert(lc != nullptr);
  assert(std::strcmp(lc->decimal_point, ".") == 0);

  // Requesting a locale that (almost certainly) doesn't exist must fail
  // without disturbing the currently installed one.
  const char* bogus = std::setlocale(LC_ALL, "definitely-not-a-real-locale");
  assert(bogus == nullptr);
  assert(std::strcmp(std::setlocale(LC_ALL, nullptr), "C") == 0);
}
