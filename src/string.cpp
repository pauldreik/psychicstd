#include <cerrno>
#include <cstdlib>
#include <string>
#include <wchar.h>

namespace std {

template <typename Value>
string format_number(const char* format, Value value) {
  char buffer[8192];
  int length = __builtin_sprintf(buffer, format, value);
  return {buffer, static_cast<size_t>(length)};
}

string to_string(int value) { return format_number("%d", value); }
string to_string(long value) { return format_number("%ld", value); }
string to_string(long long value) { return format_number("%lld", value); }
string to_string(unsigned value) { return format_number("%u", value); }
string to_string(unsigned long value) { return format_number("%lu", value); }
string to_string(unsigned long long value) {
  return format_number("%llu", value);
}
string to_string(float value) { return format_number("%f", value); }
string to_string(double value) { return format_number("%f", value); }
string to_string(long double value) { return format_number("%Lf", value); }

namespace {
template <typename Value>
wstring format_wide_number(const wchar_t* format, Value value) {
  wchar_t buffer[8192];
  int length =
      ::swprintf(buffer, sizeof(buffer) / sizeof(*buffer), format, value);
  return {buffer, static_cast<size_t>(length)};
}
} // namespace

wstring to_wstring(int value) { return format_wide_number(L"%d", value); }
wstring to_wstring(long value) { return format_wide_number(L"%ld", value); }
wstring to_wstring(long long value) {
  return format_wide_number(L"%lld", value);
}
wstring to_wstring(unsigned value) { return format_wide_number(L"%u", value); }
wstring to_wstring(unsigned long value) {
  return format_wide_number(L"%lu", value);
}
wstring to_wstring(unsigned long long value) {
  return format_wide_number(L"%llu", value);
}
wstring to_wstring(float value) { return format_wide_number(L"%f", value); }
wstring to_wstring(double value) { return format_wide_number(L"%f", value); }
wstring to_wstring(long double value) {
  return format_wide_number(L"%Lf", value);
}

#if defined(__cpp_exceptions)
template <typename Value>
Value finish_conversion(const string& input, char* end, Value value,
                        size_t* pos, const char* name) {
  if (pos)
    *pos = static_cast<size_t>(end - input.c_str());
  if (end == input.c_str())
    __throw_invalid_argument(name);
  if (errno == ERANGE)
    __throw_out_of_range(name);
  return value;
}

int stoi(const string& input, size_t* pos, int base) {
  errno = 0;
  char* end;
  long value = ::strtol(input.c_str(), &end, base);
  value = finish_conversion(input, end, value, pos, "stoi");
  if (value < (-__INT_MAX__ - 1) || value > __INT_MAX__)
    __throw_out_of_range("stoi");
  return static_cast<int>(value);
}

long stol(const string& input, size_t* pos, int base) {
  errno = 0;
  char* end;
  auto value = ::strtol(input.c_str(), &end, base);
  return finish_conversion(input, end, value, pos, "stol");
}

long long stoll(const string& input, size_t* pos, int base) {
  errno = 0;
  char* end;
  auto value = ::strtoll(input.c_str(), &end, base);
  return finish_conversion(input, end, value, pos, "stoll");
}

unsigned long stoul(const string& input, size_t* pos, int base) {
  errno = 0;
  char* end;
  auto value = ::strtoul(input.c_str(), &end, base);
  return finish_conversion(input, end, value, pos, "stoul");
}

unsigned long long stoull(const string& input, size_t* pos, int base) {
  errno = 0;
  char* end;
  auto value = ::strtoull(input.c_str(), &end, base);
  return finish_conversion(input, end, value, pos, "stoull");
}

float stof(const string& input, size_t* pos) {
  errno = 0;
  char* end;
  auto value = ::strtof(input.c_str(), &end);
  return finish_conversion(input, end, value, pos, "stof");
}

double stod(const string& input, size_t* pos) {
  errno = 0;
  char* end;
  auto value = ::strtod(input.c_str(), &end);
  return finish_conversion(input, end, value, pos, "stod");
}

long double stold(const string& input, size_t* pos) {
  errno = 0;
  char* end;
  auto value = ::strtold(input.c_str(), &end);
  return finish_conversion(input, end, value, pos, "stold");
}
#endif

} // namespace std
