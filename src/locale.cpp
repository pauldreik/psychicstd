#include <ctime>
#include <cwchar>
#include <locale>
#include <stdexcept>
#include <typeinfo>

namespace std {

void __throw_bad_cast() { _PSYCHICSTD_THROW(bad_cast()); }

namespace {

bool same_name(const char* left, const char* right) noexcept {
  while (*left && *left == *right) {
    ++left;
    ++right;
  }
  return *left == *right;
}

} // namespace

locale::locale(const char* name) : rep_(_make_named(name)) {}

bool locale::operator==(const locale& other) const noexcept {
  if (rep_ == other.rep_)
    return true;
  return rep_ && other.rep_ && rep_->name && other.rep_->name &&
         same_name(rep_->name, other.rep_->name);
}

locale::_rep::_rep(const char* locale_name) {
  size_t size = 0;
  while (locale_name[size])
    ++size;
  name = new char[size + 1];
  for (size_t i = 0; i <= size; ++i)
    name[i] = locale_name[i];
}

locale::_rep* locale::_make_named(const char* name) {
  if (!name || same_name(name, "C") || same_name(name, "POSIX"))
    return nullptr;
  // Named facets are not implemented yet. Accept the English UTF-8 locale,
  // whose behavior matches the current facets, and reject other names rather
  // than claiming semantics they do not provide.
  if (!same_name(name, "en_US.UTF-8"))
    _PSYCHICSTD_THROW_HELPER(__throw_runtime_error,
                             "locale name not supported");
  return new _rep(name);
}

void swap(locale& left, locale& right) noexcept {
  locale::_rep* rep = left.rep_;
  left.rep_ = right.rep_;
  right.rep_ = rep;
}

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
