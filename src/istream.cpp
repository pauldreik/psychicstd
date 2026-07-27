#include <cerrno>
#include <cstdlib>
#include <istream>

namespace std {

long long ios_base::_parse_signed(const char* input, int base) {
  return ::strtoll(input, nullptr, base);
}

unsigned long long ios_base::_parse_unsigned(const char* input, int base) {
  return ::strtoull(input, nullptr, base);
}

template <typename Float, typename Parse>
bool parse_float(const char* input, Float& value, int& consumed, Parse parse) {
  char* end = nullptr;
  errno = 0;
  value = parse(input, &end);
  consumed = static_cast<int>(end - input);
  return end != input &&
         (errno != ERANGE || (value != 0 && __builtin_isfinite(value)));
}

bool ios_base::_parse_float(const char* input, float& value, int& consumed) {
  return parse_float(input, value, consumed, ::strtof);
}

bool ios_base::_parse_float(const char* input, double& value, int& consumed) {
  return parse_float(input, value, consumed, ::strtod);
}

bool ios_base::_parse_float(const char* input, long double& value,
                            int& consumed) {
  return parse_float(input, value, consumed, ::strtold);
}

template <typename Char, typename Traits>
basic_istream<Char, Traits>&
basic_istream<Char, Traits>::extract_chars(Char* value, size_t size) {
  sentry s(*this);
  if (!s) {
    this->setstate(ios_base::failbit);
    return *this;
  }
  size_t limit = size - 1;
  if (this->width() > 0 && static_cast<size_t>(this->width()) - 1 < limit)
    limit = static_cast<size_t>(this->width()) - 1;
  this->width(0);
  size_t count = 0;
  auto* sb = this->rdbuf();
  int_type c;
  while (count < limit &&
         !Traits::eq_int_type(c = sb->sgetc(), Traits::eof())) {
    Char ch = Traits::to_char_type(c);
    if (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r')
      break;
    value[count++] = ch;
    sb->sbumpc();
  }
  value[count] = Char();
  if (count == 0)
    this->setstate(ios_base::failbit);
  return *this;
}

template class basic_istream<char, char_traits<char>>;

} // namespace std
