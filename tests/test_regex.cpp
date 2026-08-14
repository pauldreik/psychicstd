#include "psyassert.h"
#include <regex>
#include <string>
#include <utility>
#include <vector>

namespace {

void test_basic_matching_and_replacement() {
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
}

void test_errors() {
  bool threw = false;
  try {
    (void)std::regex("*");
  } catch (const std::runtime_error&) {
    threw = true;
  }
  psyassert(threw);

  bool threw_regex_error = false;
  try {
    (void)std::regex("a(b");
  } catch (const std::regex_error& e) {
    threw_regex_error = true;
    psyassert(e.code() == std::regex_constants::error_paren);
  }
  psyassert(threw_regex_error);
}

#if !defined(__APPLE__) || defined(PSYCHICSTD_TEST_PSYCHICSTD)
void test_invalid_character_class_ranges() {
  const char* patterns[] = {"([\\w-a])", "([a-\\w])", "([w-a])"};
  for (const char* pattern : patterns) {
    bool threw = false;
    try {
      (void)std::regex(pattern);
    } catch (const std::regex_error& error) {
      threw = true;
      psyassert(error.code() == std::regex_constants::error_range);
    }
    psyassert(threw);
  }
}
#endif

void test_match_results() {
  const std::string haystack = "before-MATCH-after";
  std::smatch parts;
  psyassert(std::regex_search(haystack, parts, std::regex("MATCH")));
  psyassert(parts.prefix().str() == "before-");
  psyassert(parts.suffix().str() == "-after");
  psyassert(parts.prefix().matched);
  psyassert(parts.suffix().matched);

  const std::string whole_subject = "MATCH";
  psyassert(std::regex_search(whole_subject, parts, std::regex("MATCH")));
  psyassert(parts.prefix().str().empty());
  psyassert(parts.suffix().str().empty());
  psyassert(!parts.prefix().matched);
  psyassert(!parts.suffix().matched);

  const std::string missing_subject = "missing";
  psyassert(!std::regex_search(missing_subject, parts, std::regex("MATCH")));
  psyassert(parts.prefix().str().empty());
  psyassert(parts.suffix().str().empty());
  psyassert(!parts.prefix().matched);
  psyassert(!parts.suffix().matched);

  std::smatch no_submatches;
  const std::regex no_subexpression_results("(M)(ATCH)",
                                            std::regex_constants::nosubs);
  psyassert(no_subexpression_results.mark_count() == 0);
  psyassert(std::regex_search(whole_subject, no_submatches,
                              no_subexpression_results));
  psyassert(no_submatches.size() == 1);
  psyassert(no_submatches.str() == "MATCH");
}

void test_copy_and_move() {
  std::regex original("[0-9]+");
  std::regex copy = original;
  psyassert(std::regex_match("123", copy));
  psyassert(std::regex_match("123", original));
  std::regex assigned("placeholder");
  assigned = original;
  psyassert(std::regex_match("456", assigned));
  std::regex moved = std::move(copy);
  psyassert(std::regex_match("789", moved));
}

void test_noncapturing_groups() {
  psyassert(std::regex_search("123ms", std::regex("(?:\\d+)(?:ms|us)")));
  psyassert(!std::regex_search("123xy", std::regex("(?:\\d+)(?:ms|us)")));

  std::smatch mixed;
  const std::string mixed_subject = "abc123";
  psyassert(
      std::regex_match(mixed_subject, mixed, std::regex("(?:[a-z]+)([0-9]+)")));
  psyassert(mixed.size() == 2);
  psyassert(mixed[1].str() == "123");
  psyassert(std::regex_match("abcabc", std::regex("(?:x)?(abc)\\1")));
  psyassert(std::regex_replace("abc123", std::regex("(?:[a-z]+)([0-9]+)"),
                               "$1") == "123");
}

void test_sub_match_members() {
  std::smatch member_match;
  const std::string member_subject = "ac";
  psyassert(
      std::regex_match(member_subject, member_match, std::regex("(a)(x)?(c)")));
  psyassert(member_match[1].matched);
  psyassert(!member_match[2].matched);
  psyassert(member_match[3].matched);
  {
    std::smatch optional_match;
    const std::string timestamp_subject = "2024-01-02 03:04:05.123456";
    psyassert(std::regex_match(
        timestamp_subject, optional_match,
        std::regex(
            R"((\d{4})-(\d{2})-(\d{2})[ T](\d{2}):(\d{2}):(\d{2})(?:\.(\d{1,6}))?(?:([+-]\d{2}:\d{2}|Z))?)")));
    psyassert(optional_match[7].str() == "123456");
  }
}

void test_escaped_character_classes() {
  psyassert(std::regex_search("MyCourt-42", std::regex("[A-Za-z0-9\\-\\_]+")));
  {
    std::smatch hyphen_match;
    const std::string hyphen_subject = "MyCourt-42";
    psyassert(std::regex_match(hyphen_subject, hyphen_match,
                               std::regex("[A-Za-z0-9\\-\\_]+")));
    psyassert(hyphen_match.str(0) == "MyCourt-42");
  }
  {
    std::smatch full_path_match;
    const std::string full_path_subject = "sports/tennis/courts/MyCourt-42";
    psyassert(std::regex_search(
        full_path_subject, full_path_match,
        std::regex("sports/tennis/courts/([A-Za-z0-9\\-\\_]+)$")));
    psyassert(full_path_match[1].str() == "MyCourt-42");
  }
  psyassert(std::regex_match(std::string("a_b-c"), std::regex("[a\\_b\\-c]+")));
  psyassert(std::regex_match(std::string("a-]"), std::regex("[a\\-\\]]+")));
}

}

int main() {
  test_basic_matching_and_replacement();
  test_errors();
#if !defined(__APPLE__) || defined(PSYCHICSTD_TEST_PSYCHICSTD)
  test_invalid_character_class_ranges();
#endif
  test_match_results();
  test_copy_and_move();
  test_noncapturing_groups();
  test_sub_match_members();
  test_escaped_character_classes();
}
