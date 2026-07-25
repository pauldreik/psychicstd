#include <ctime>
#include <cwchar>
#include <locale>

namespace std::__locale_detail {

size_t format_time(char* text, size_t size, const char* format,
                   const ::tm* time) {
  return ::strftime(text, size, format, time);
}

size_t format_time(wchar_t* text, size_t size, const wchar_t* format,
                   const ::tm* time) {
  return ::wcsftime(text, size, format, time);
}

} // namespace std::__locale_detail
