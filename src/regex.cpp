#include <regex>

namespace std {

regex::regex() : valid_(false) {}

regex::regex(const string& pattern, flag_type flags)
    : regex(pattern.c_str(), flags) {}

regex::regex(const char* pattern, flag_type flags) {
  int native_flags = REG_EXTENDED;
  if (flags & icase)
    native_flags |= REG_ICASE;
  if (::regcomp(&re_, pattern, native_flags) != 0)
    _PSYCHICSTD_THROW_HELPER(__throw_runtime_error, "regex: bad pattern");
  valid_ = true;
}

regex::~regex() {
  if (valid_)
    ::regfree(&re_);
}

regex::regex(regex&& other) noexcept : re_(other.re_), valid_(other.valid_) {
  other.valid_ = false;
}

string smatch::str(size_t i) const {
  return i < matches_.size() ? matches_[i] : string{};
}

void smatch::set(const string& subject, const ::regmatch_t* matches, size_t n) {
  matches_.clear();
  ready_ = true;
  for (size_t i = 0; i < n; ++i) {
    if (matches[i].rm_so >= 0)
      matches_.push_back(subject.substr(
          static_cast<size_t>(matches[i].rm_so),
          static_cast<size_t>(matches[i].rm_eo - matches[i].rm_so)));
    else
      matches_.push_back(string{});
  }
}

bool regex_search(const string& subject, smatch& match,
                  const regex& expression) {
  if (!expression.valid())
    return false;
  static constexpr size_t match_count = 16;
  ::regmatch_t matches[match_count];
  if (::regexec(&expression.native(), subject.c_str(), match_count, matches,
                0) == 0) {
    match.set(subject, matches, match_count);
    return true;
  }
  return false;
}

bool regex_search(const char* subject, smatch& match, const regex& expression) {
  return regex_search(string(subject), match, expression);
}

bool regex_search(const string& subject, const regex& expression) {
  smatch match;
  return regex_search(subject, match, expression);
}

bool regex_search(const char* subject, const regex& expression) {
  smatch match;
  return regex_search(string(subject), match, expression);
}

bool regex_match(const string& subject, smatch& match,
                 const regex& expression) {
  return regex_search(subject, match, expression) && !match.empty() &&
         match.str(0) == subject;
}

bool regex_match(const string& subject, const regex& expression) {
  smatch match;
  return regex_match(subject, match, expression);
}

bool regex_match(const char* subject, const regex& expression) {
  return regex_match(string(subject), expression);
}

} // namespace std
