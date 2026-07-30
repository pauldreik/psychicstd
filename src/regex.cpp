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
    out.push_back('\\');
    out.push_back(escaped);
    break;
  }
}

string translate_ecmascript(const char* pattern) {
  string result;

  for (const char* p = pattern; *p; ++p) {
    if (*p == '.') {
      result.append("[^\n\r]");
      continue;
    }

    if (*p == '[') {
      string contents;
      bool includes_closing_bracket = false;
      const char* q = p + 1;
      if (*q == '^')
        ++q;

      for (; *q && *q != ']'; ++q) {
        if (*q != '\\' || !q[1]) {
          contents.push_back(*q);
        } else if (*++q == ']') {
          includes_closing_bracket = true;
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
    append_escape(result, *++p, false);
  }
  return result;
}

} // namespace

regex::regex() : valid_(false) {}

regex::regex(const string& pattern, flag_type flags)
    : regex(pattern.c_str(), flags) {}

regex::regex(const char* pattern, flag_type flags) {
  int native_flags = REG_EXTENDED;
  if (flags & icase)
    native_flags |= REG_ICASE;
  const string translated =
      (flags & extended) ? string(pattern) : translate_ecmascript(pattern);
  if (::regcomp(&re_, translated.c_str(), native_flags) != 0)
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

string regex_replace(const string& subject, const regex& expression,
                     const string& replacement) {
  if (!expression.valid())
    return subject;

  string result;
  size_t offset = 0;
  static constexpr size_t match_count = 10;
  ::regmatch_t matches[match_count];
  while (offset <= subject.size() &&
         ::regexec(&expression.native(), subject.c_str() + offset, match_count,
                   matches, offset ? REG_NOTBOL : 0) == 0) {
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
      } else if (marker >= '0' && marker <= '9' &&
                 matches[marker - '0'].rm_so >= 0) {
        const auto& match = matches[marker - '0'];
        result.append(subject.data() + offset + match.rm_so,
                      match.rm_eo - match.rm_so);
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

} // namespace std
