#include "psyassert.h"
#include <regex>
#include <string>
#include <vector>

int main() {
  std::regex re("hello");
  psyassert(std::regex_match("hello", re));
  const std::string text = "say hello";
  psyassert(std::regex_search(text.begin(), text.end(), re));
  const std::string search_text = "say hello";
  std::smatch search_match;
  psyassert(std::regex_search(search_text, search_match, re));
  psyassert(search_match.size() == 1);

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

  std::basic_regex<char> groups("(-)?(0x)?([0-9a-zA-Z]+)");
  std::smatch group_match;
  const std::string grouped_value = "-0xff";
  psyassert(std::regex_match(grouped_value, group_match, groups));
  psyassert(group_match.length() == 5);
  psyassert(group_match.length(1) == 1);
  psyassert(group_match[2].str() == "0x");
  psyassert(static_cast<std::string>(group_match[3]) == "ff");

  std::smatch partial_match;
  const std::string partial_value = "--a";
  psyassert(!std::regex_match(partial_value, partial_match,
                              std::regex("--([[:alnum:]][-_[:alnum:]]+)|-"
                                         "([[:alnum:]].*)")));
  psyassert(partial_match.empty());
  psyassert(partial_match.ready());

  psyassert(std::regex_search(grouped_value, group_match, groups));
  const std::string non_number = "not a number";
  const std::regex digits("^[0-9]+$");
  psyassert(!std::regex_search(non_number, group_match, digits));
  psyassert(group_match.empty());
  psyassert(group_match.ready());

  std::match_results<const char*> pointer_match;
  psyassert(std::regex_match("--option=value", pointer_match,
                             std::basic_regex<char>("--([^=]+)=(.*)")));
  psyassert(pointer_match[1].str() == "option");
  psyassert(pointer_match[2].str() == "value");

  const std::string options = "alpha, beta,gamma";
  const std::regex separator(", *");
  std::vector<std::string> tokens;
  for (std::sregex_token_iterator
           iterator(options.begin(), options.end(), separator, -1),
       end;
       iterator != end; ++iterator)
    tokens.push_back(*iterator);
  psyassert(tokens == (std::vector<std::string>{"alpha", "beta", "gamma"}));

  const std::string two_letters = "ab";
  const std::regex empty_separator("");
  std::vector<std::string> zero_length_tokens(
      std::sregex_token_iterator(two_letters.begin(), two_letters.end(),
                                 empty_separator, -1),
      std::sregex_token_iterator());
  psyassert(zero_length_tokens == (std::vector<std::string>{"", "a", "b"}));

  const std::string empty_subject;
  const std::regex comma(",");
  std::vector<std::string> empty_subject_tokens(
      std::sregex_token_iterator(empty_subject.begin(), empty_subject.end(),
                                 comma, -1),
      std::sregex_token_iterator());
  psyassert(empty_subject_tokens == (std::vector<std::string>{""}));

  bool threw = false;
  try {
    (void)std::regex("*");
  } catch (const std::runtime_error&) {
    threw = true;
  }
  psyassert(threw);
}
