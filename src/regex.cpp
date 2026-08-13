#include <regex>

namespace std {

namespace {

// Keep ECMAScript adaptation out of the public header's parse path.
void append_escape(string& out, char escaped, bool in_class) {
  switch (escaped) {
  case 'n':
    out.push_back('\n');
    break;
  case 'r':
    out.push_back('\r');
    break;
  case 't':
    out.push_back('\t');
    break;
  case 'f':
    out.push_back('\f');
    break;
  case 'v':
    out.push_back('\v');
    break;
  case 'd':
    out.append(in_class ? "[:digit:]" : "[[:digit:]]");
    break;
  case 's':
    out.append(in_class ? "[:space:]" : "[[:space:]]");
    break;
  case 'w':
    out.append(in_class ? "[:alnum:]_" : "[[:alnum:]_]");
    break;
  case 'D':
    if (!in_class) {
      out.append("[^[:digit:]]");
      return;
    }
    [[fallthrough]];
  case 'S':
    if (!in_class) {
      out.append("[^[:space:]]");
      return;
    }
    [[fallthrough]];
  case 'W':
    if (!in_class) {
      out.append("[^[:alnum:]_]");
      return;
    }
    [[fallthrough]];
  default:
    // Outside a class, POSIX ERE still needs the backslash to keep an
    // escaped metacharacter (\., \(, \+, ...) literal. Inside a class,
    // backslash has no escaping power at all in POSIX bracket expressions --
    // it's just an ordinary character, so re-emitting it here would inject a
    // spurious literal backslash into the class instead of just the intended
    // character. (Escaped '-' is handled separately by the caller, since a
    // literal '-' also needs repositioning, not just de-escaping -- see
    // translate_ecmascript.)
    if (in_class) {
      out.push_back(escaped);
    } else {
      out.push_back('\\');
      out.push_back(escaped);
    }
    break;
  }
}

string translate_ecmascript(const char* pattern, vector<size_t>& submatches) {
  // Apple's regcomp rejects an empty pattern even though ECMAScript accepts it.
  if (!*pattern)
    return "x{0}";

  string result;
  size_t native_submatches = 0;

  for (const char* p = pattern; *p; ++p) {
    if (*p == '.') {
      result.append("[^\n\r]");
      continue;
    }

    if (*p == '(' && p[1] == '?' && p[2] == ':') {
      // ECMAScript non-capturing group: POSIX ERE has no such concept, but a
      // plain capturing group matches identically (it only shifts numbered
      // backreferences/submatches relative to real ECMAScript semantics).
      result.push_back('(');
      ++native_submatches;
      p += 2;
      continue;
    }

    if (*p == '(') {
      result.push_back(*p);
      submatches.push_back(++native_submatches);
      continue;
    }

    if (*p == '[') {
      string contents;
      // A literal '-' can only be trusted to stay literal (not become a
      // range operator) at the very first or last position of a POSIX
      // bracket expression. An escaped '\-' from the ECMAScript source
      // sitting anywhere else in `contents` -- e.g. between two other
      // escaped/plain members, as in "[A-Za-z0-9\-\_]" -- would otherwise be
      // parsed by POSIX as ordinary-char-'-'-ordinary-char, i.e. a range,
      // silently dropping the intended literal hyphen from the class
      // entirely (confirmed: "[A-Za-z0-9\-\_]+" against "MyCourt-42" matched
      // only "MyCourt", stopping right where the hyphen should have been
      // accepted). Deferred hyphens are appended at the very end below,
      // which is always a safe literal position.
      bool includes_literal_hyphen = false;
      bool includes_closing_bracket = false;
      const char* q = p + 1;
      if (*q == '^')
        ++q;

      for (; *q && *q != ']'; ++q) {
        if (*q != '\\' || !q[1]) {
          contents.push_back(*q);
        } else if (*++q == ']') {
          includes_closing_bracket = true;
        } else if (*q == '-') {
          includes_literal_hyphen = true;
        } else {
          append_escape(contents, *q, true);
        }
      }

      result.push_back('[');
      if (p[1] == '^')
        result.push_back('^');
      if (includes_closing_bracket)
        result.push_back(']');
      result.append(contents);
      if (includes_literal_hyphen)
        result.push_back('-');
      if (*q == ']') {
        result.push_back(']');
        p = q;
      }
      continue;
    }

    if (*p != '\\' || !p[1]) {
      result.push_back(*p);
      continue;
    }
    const char escaped = *++p;
    if (escaped >= '1' && escaped <= '9') {
      const size_t logical = static_cast<size_t>(escaped - '0');
      if (logical <= submatches.size()) {
        result.push_back('\\');
        result.push_back(static_cast<char>('0' + submatches[logical - 1]));
        continue;
      }
    }
    append_escape(result, escaped, false);
  }
  return result;
}

}

namespace {

// POSIX regcomp() error codes have no fixed numeric mapping to
// std::regex_constants::error_type; approximate the closest named category.
regex_constants::error_type to_error_type(int posix_code) {
  switch (posix_code) {
  case REG_ECOLLATE:
    return regex_constants::error_collate;
  case REG_ECTYPE:
    return regex_constants::error_ctype;
  case REG_EESCAPE:
    return regex_constants::error_escape;
  case REG_ESUBREG:
    return regex_constants::error_backref;
  case REG_EBRACK:
    return regex_constants::error_brack;
  case REG_EPAREN:
#ifdef REG_ERPAREN
  case REG_ERPAREN:
#endif
    return regex_constants::error_paren;
  case REG_EBRACE:
    return regex_constants::error_brace;
  case REG_BADBR:
    return regex_constants::error_badbrace;
  case REG_ERANGE:
    return regex_constants::error_range;
  case REG_ESPACE:
    return regex_constants::error_space;
  case REG_BADRPT:
    return regex_constants::error_badrepeat;
#ifdef REG_ESIZE
  case REG_ESIZE:
    return regex_constants::error_complexity;
#endif
  default:
    return regex_constants::error_stack;
  }
}

}

regex_error::regex_error(regex_constants::error_type code)
    : runtime_error("regex_error"), code_(code) {}

regex_constants::error_type regex_error::code() const { return code_; }

regex::regex() = default;

regex::regex(const string& pattern, flag_type flags)
    : regex(pattern.c_str(), flags) {}

regex::regex(const char* pattern, flag_type flags)
    : pattern_(pattern), flags_(flags) {
  compile();
}

void regex::compile() {
  submatches_.clear();
  int native_flags = REG_EXTENDED;
#ifdef REG_ENHANCED
  native_flags |= REG_ENHANCED;
#endif
  if (flags_ & icase)
    native_flags |= REG_ICASE;
  string translated;
  if (flags_ & extended) {
    translated = pattern_;
  } else {
    translated = translate_ecmascript(pattern_.c_str(), submatches_);
  }
  if (int err = ::regcomp(&re_, translated.c_str(), native_flags); err != 0)
    _PSYCHICSTD_THROW(regex_error(to_error_type(err)));
  valid_ = true;
  if (flags_ & nosubs) {
    submatches_.clear();
  } else if (flags_ & extended) {
    for (size_t i = 1; i <= re_.re_nsub; ++i)
      submatches_.push_back(i);
  }
}

regex::~regex() {
  if (valid_)
    ::regfree(&re_);
}

regex::regex(const regex& other)
    : pattern_(other.pattern_), flags_(other.flags_) {
  if (other.valid_)
    compile();
}

regex& regex::operator=(const regex& other) {
  if (this == &other)
    return *this;
  if (valid_) {
    ::regfree(&re_);
    valid_ = false;
  }
  pattern_ = other.pattern_;
  flags_ = other.flags_;
  if (other.valid_)
    compile();
  return *this;
}

regex::regex(regex&& other) noexcept
    : re_(other.re_), valid_(other.valid_),
      pattern_(static_cast<string&&>(other.pattern_)), flags_(other.flags_),
      submatches_(static_cast<vector<size_t>&&>(other.submatches_)) {
  other.valid_ = false;
}

regex& regex::operator=(regex&& other) noexcept {
  if (this == &other)
    return *this;
  if (valid_)
    ::regfree(&re_);
  re_ = other.re_;
  valid_ = other.valid_;
  pattern_ = static_cast<string&&>(other.pattern_);
  flags_ = other.flags_;
  submatches_ = static_cast<vector<size_t>&&>(other.submatches_);
  other.valid_ = false;
  return *this;
}

size_t regex::native_submatch(size_t logical) const noexcept {
  if (logical == 0)
    return 0;
  return logical <= submatches_.size() ? submatches_[logical - 1]
                                       : re_.re_nsub + 1;
}

size_t regex::mark_count() const noexcept { return submatches_.size(); }

__regex_sub_match::__regex_sub_match(string value, bool didMatch)
    : matched(didMatch), value_(static_cast<string&&>(value)) {}

string __regex_sub_match::str() const { return value_; }

size_t __regex_sub_match::length() const noexcept { return value_.size(); }

__regex_sub_match::operator string() const { return value_; }

string smatch::str(size_t i) const {
  return i < matches_.size() ? matches_[i].str() : string{};
}

size_t smatch::length(size_t i) const noexcept {
  return i < matches_.size() ? matches_[i].length() : 0;
}

const __regex_sub_match& smatch::operator[](size_t i) const noexcept {
  static const __regex_sub_match unmatched;
  return i < matches_.size() ? matches_[i] : unmatched;
}

size_t smatch::size() const noexcept { return matches_.size(); }

bool smatch::empty() const noexcept { return matches_.empty(); }

bool smatch::ready() const noexcept { return ready_; }

const __regex_sub_match& smatch::prefix() const noexcept { return prefix_; }

const __regex_sub_match& smatch::suffix() const noexcept { return suffix_; }

void smatch::set(const string& subject, const ::regmatch_t* matches, size_t n) {
  matches_.clear();
  ready_ = true;
  for (size_t i = 0; i < n; ++i) {
    if (matches[i].rm_so >= 0)
      matches_.emplace_back(
          subject.substr(
              static_cast<size_t>(matches[i].rm_so),
              static_cast<size_t>(matches[i].rm_eo - matches[i].rm_so)),
          true);
    else
      matches_.emplace_back();
  }
  if (n > 0 && matches[0].rm_so >= 0) {
    const auto match_begin = static_cast<size_t>(matches[0].rm_so);
    const auto match_end = static_cast<size_t>(matches[0].rm_eo);
    prefix_ =
        __regex_sub_match(subject.substr(0, match_begin), match_begin != 0);
    suffix_ = __regex_sub_match(subject.substr(match_end),
                                match_end != subject.size());
  } else {
    prefix_ = __regex_sub_match(string(), false);
    suffix_ = __regex_sub_match(string(), false);
  }
}

bool regex_search(const string& subject, smatch& match,
                  const regex& expression) {
  if (!expression.valid())
    return false;
  vector<::regmatch_t> native_matches(expression.native().re_nsub + 1);
  if (::regexec(&expression.native(), subject.c_str(), native_matches.size(),
                native_matches.data(), 0) == 0) {
    vector<::regmatch_t> matches(expression.mark_count() + 1);
    for (size_t i = 0; i < matches.size(); ++i)
      matches[i] = native_matches[expression.native_submatch(i)];
    match.set(subject, matches.data(), matches.size());
    return true;
  }
  match.set(subject, nullptr, 0);
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
  if (regex_search(subject, match, expression) && !match.empty() &&
      match.str(0) == subject)
    return true;
  match.set(subject, nullptr, 0);
  return false;
}

bool regex_match(const char* subject, cmatch& match, const regex& expression) {
  return regex_match(string(subject), static_cast<smatch&>(match), expression);
}

bool regex_match(const string& subject, const regex& expression) {
  smatch match;
  return regex_match(subject, match, expression);
}

bool regex_match(const char* subject, const regex& expression) {
  return regex_match(string(subject), expression);
}

string regex_replace(const string& subject, const regex& expression,
                     const string& replacement) {
  if (!expression.valid())
    return subject;

  string result;
  size_t offset = 0;
  vector<::regmatch_t> matches(expression.native().re_nsub + 1);
  while (offset <= subject.size() &&
         ::regexec(&expression.native(), subject.c_str() + offset,
                   matches.size(), matches.data(),
                   offset ? REG_NOTBOL : 0) == 0) {
    const size_t match_begin = offset + matches[0].rm_so;
    const size_t match_end = offset + matches[0].rm_eo;
    result.append(subject.data() + offset, match_begin - offset);
    for (size_t i = 0; i < replacement.size(); ++i) {
      if (replacement[i] != '$' || i + 1 == replacement.size()) {
        result.push_back(replacement[i]);
        continue;
      }
      const char marker = replacement[++i];
      if (marker == '$') {
        result.push_back('$');
      } else if (marker == '&') {
        result.append(subject.data() + match_begin, match_end - match_begin);
      } else if (marker >= '0' && marker <= '9') {
        const size_t logical = static_cast<size_t>(marker - '0');
        if (logical <= expression.mark_count()) {
          const auto& match = matches[expression.native_submatch(logical)];
          if (match.rm_so >= 0)
            result.append(subject.data() + offset + match.rm_so,
                          match.rm_eo - match.rm_so);
        }
      } else {
        result.push_back('$');
        result.push_back(marker);
      }
    }
    offset = match_end;
    if (matches[0].rm_so == matches[0].rm_eo) {
      if (offset == subject.size())
        break;
      result.push_back(subject[offset++]);
    }
  }
  result.append(subject.data() + offset, subject.size() - offset);
  return result;
}

sregex_token_iterator::sregex_token_iterator(string::const_iterator first,
                                             string::const_iterator last,
                                             const regex& expression,
                                             int submatch) {
  if (!expression.valid())
    return;
  string subject(first, last);
  size_t token_start = 0;
  size_t search_offset = 0;
  bool found_match = false;
  vector<::regmatch_t> matches(expression.native().re_nsub + 1);
  while (search_offset <= subject.size() &&
         ::regexec(&expression.native(), subject.c_str() + search_offset,
                   matches.size(), matches.data(),
                   search_offset ? REG_NOTBOL : 0) == 0) {
    found_match = true;
    const size_t match_begin = search_offset + matches[0].rm_so;
    const size_t match_end = search_offset + matches[0].rm_eo;
    if (submatch == -1) {
      tokens_.push_back(subject.substr(token_start, match_begin - token_start));
    } else if (submatch >= 0 &&
               static_cast<size_t>(submatch) <= expression.mark_count() &&
               matches[expression.native_submatch(submatch)].rm_so >= 0) {
      const auto& match = matches[expression.native_submatch(submatch)];
      tokens_.push_back(
          subject.substr(search_offset + match.rm_so,
                         static_cast<size_t>(match.rm_eo - match.rm_so)));
    } else {
      tokens_.emplace_back();
    }
    token_start = match_end;
    search_offset = match_end;
    if (match_begin == match_end) {
      if (search_offset == subject.size())
        break;
      ++search_offset;
    }
  }
  if (submatch == -1 && (token_start < subject.size() || !found_match))
    tokens_.push_back(subject.substr(token_start));
}

sregex_token_iterator::reference
sregex_token_iterator::operator*() const noexcept {
  return tokens_[position_];
}

sregex_token_iterator::pointer
sregex_token_iterator::operator->() const noexcept {
  return &tokens_[position_];
}

sregex_token_iterator& sregex_token_iterator::operator++() {
  ++position_;
  return *this;
}

sregex_token_iterator sregex_token_iterator::operator++(int) {
  sregex_token_iterator previous = *this;
  ++*this;
  return previous;
}

bool operator==(const sregex_token_iterator& left,
                const sregex_token_iterator& right) noexcept {
  const bool left_end = left.position_ == left.tokens_.size();
  const bool right_end = right.position_ == right.tokens_.size();
  if (left_end || right_end)
    return left_end == right_end;
  return left.position_ == right.position_ && left.tokens_ == right.tokens_;
}

}
