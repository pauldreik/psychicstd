#include "psyassert.h"
#include <regex>

int main() {
  std::regex re("hello");
  psyassert(std::regex_match("hello", re));
  const std::string text = "say hello";
  psyassert(std::regex_search(text.begin(), text.end(), re));

  std::regex insensitive("HELLO", std::regex_constants::icase);
  psyassert(std::regex_search(text.begin(), text.end(), insensitive,
                              std::regex_constants::match_default));

  bool threw = false;
  try {
    (void)std::regex("*");
  } catch (const std::runtime_error&) {
    threw = true;
  }
  psyassert(threw);
}
