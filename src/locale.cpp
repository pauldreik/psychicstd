#include <ctime>
#include <cwchar>
#include <locale>
#include <typeinfo>

namespace std {

void __throw_bad_cast() { _PSYCHICSTD_THROW(bad_cast()); }

namespace __locale_detail {

size_t format_time(char* text, size_t size, const char* format,
                   const ::tm* time) {
  return ::strftime(text, size, format, time);
}

size_t format_time(wchar_t* text, size_t size, const wchar_t* format,
                   const ::tm* time) {
  return ::wcsftime(text, size, format, time);
}

} // namespace __locale_detail
} // namespace std
