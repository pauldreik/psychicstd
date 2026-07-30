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

  psyassert(std::regex_search("first\nsecond", std::regex("first\\nsecond")));
  psyassert(!std::regex_search("\n", std::regex(".")));
  psyassert(std::regex_search("a1 b2", std::regex("\\w\\d\\s\\w\\d")));
  psyassert(std::regex_search("a1", std::regex("[\\w]+")));
  psyassert(!std::regex_search("abc", std::regex("^\\d+$")));
  psyassert(std::regex_search("abc", std::regex("^\\D+$")));
  psyassert(std::regex_search("]", std::regex("[a\\]]")));
  psyassert(std::regex_search("/tmp/a-b.cpp:12",
                              std::regex("[A-Za-z0-9_ ./:\\]*:[0-9]*.*")));
  psyassert(std::regex_replace("one two one", std::regex("one"), "1") ==
            "1 two 1");
  psyassert(std::regex_replace("ab", std::regex("(a)(b)"), "$2$1") == "ba");
  psyassert(std::regex_replace("b", std::regex("(a)?b"), "$1").empty());

  bool threw = false;
  try {
    (void)std::regex("*");
  } catch (const std::runtime_error&) {
    threw = true;
  }
  psyassert(threw);
}
