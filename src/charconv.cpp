#include <cerrno>
#include <charconv>
#include <cstdlib>

namespace std {

namespace {

// strtod/strtof/strtold need a NUL-terminated string; [first, last) isn't
// necessarily one, so copy into a local buffer first, matching the pattern
// already used elsewhere in psychicstd (e.g. time_get) for the same reason.
template <typename T, typename Strto>
from_chars_result parse_float(const char* first, const char* last, T& value,
                              chars_format fmt, Strto strto) {
  if (first == last || *first == '+' || *first == ' ' ||
      (*first >= '\t' && *first <= '\r'))
    return {first, errc::invalid_argument};

  const char* number = *first == '-' ? first + 1 : first;
  if (fmt == chars_format::hex && last - number >= 2 && number[0] == '0' &&
      (number[1] == 'x' || number[1] == 'X')) {
    value = *first == '-' ? -T(0) : T(0);
    return {number + 1, errc{}};
  }
  if (fmt == chars_format::hex) {
    const auto is_hex_digit = [](char c) {
      return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
             (c >= 'A' && c <= 'F');
    };
    if (number == last ||
        (!is_hex_digit(*number) &&
         !(*number == '.' && number + 1 != last && is_hex_digit(number[1]))))
      return {first, errc::invalid_argument};
  }

  const char* parse_last = last;
  if (fmt == chars_format::fixed) {
    for (const char* p = first; p != last; ++p) {
      if (*p == 'e' || *p == 'E') {
        parse_last = p;
        break;
      }
    }
  }

  const size_t n = static_cast<size_t>(parse_last - first);
  const bool hex = fmt == chars_format::hex;
  char* buf = static_cast<char*>(::malloc(n + (hex ? 3 : 1)));
  if (!buf)
    return {first, errc::result_out_of_range};
  size_t prefix = 0;
  if (hex && *first == '-')
    buf[prefix++] = '-';
  if (hex) {
    buf[prefix++] = '0';
    buf[prefix++] = 'x';
  }
  const size_t source_offset = hex && *first == '-' ? 1 : 0;
  for (size_t i = source_offset; i < n; ++i)
    buf[prefix++] = first[i];
  buf[prefix] = '\0';

  errno = 0;
  char* end = nullptr;
  T parsed = strto(buf, &end);
  if (end == buf) {
    ::free(buf);
    return {first, errc::invalid_argument};
  }
  const char* result_ptr = first + (end - buf) - (hex ? 2 : 0);
  bool has_exponent = false;
  for (const char* p = first; p != result_ptr; ++p)
    has_exponent = has_exponent || *p == 'e' || *p == 'E';
  if (fmt == chars_format::scientific && !has_exponent) {
    ::free(buf);
    return {first, errc::invalid_argument};
  }
  if (fmt != chars_format::hex) {
    if (last - number >= 2 && number[0] == '0' &&
        (number[1] == 'x' || number[1] == 'X')) {
      ::free(buf);
      return {first, errc::invalid_argument};
    }
  }
  const bool out_of_range = errno == ERANGE;
  ::free(buf);
  if (out_of_range)
    return {result_ptr, errc::result_out_of_range};
  value = parsed;
  return {result_ptr, errc{}};
}

} // namespace

from_chars_result from_chars(const char* first, const char* last, float& value,
                             chars_format fmt) noexcept {
  return parse_float(first, last, value, fmt, [](const char* s, char** end) {
    return ::strtof(s, end);
  });
}

from_chars_result from_chars(const char* first, const char* last, double& value,
                             chars_format fmt) noexcept {
  return parse_float(first, last, value, fmt, [](const char* s, char** end) {
    return ::strtod(s, end);
  });
}

from_chars_result from_chars(const char* first, const char* last,
                             long double& value, chars_format fmt) noexcept {
  return parse_float(first, last, value, fmt, [](const char* s, char** end) {
    return ::strtold(s, end);
  });
}

} // namespace std
