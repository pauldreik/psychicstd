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

template class basic_istream<char, char_traits<char>>;

} // namespace std
