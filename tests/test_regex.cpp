#include "psyassert.h"
#include <regex>

int main() {
  std::regex re("hello");
  psyassert(std::regex_match("hello", re));

  bool threw = false;
  try {
    (void)std::regex("*");
  } catch (const std::runtime_error&) {
    threw = true;
  }
  psyassert(threw);
}
