#include <ios>
#include <locale>

namespace std {

int ios_base::xalloc() {
  static int next;
  return __atomic_fetch_add(&next, 1, __ATOMIC_RELAXED);
}

ios_base::fmtflags ios_base::flags(fmtflags f) {
  fmtflags old = flags_;
  flags_ = f;
  return old;
}

ios_base::fmtflags ios_base::setf(fmtflags f) {
  fmtflags old = flags_;
  flags_ |= f;
  return old;
}

ios_base::fmtflags ios_base::setf(fmtflags f, fmtflags mask) {
  fmtflags old = flags_;
  flags_ = (flags_ & ~mask) | (f & mask);
  return old;
}

streamsize ios_base::width(streamsize w) {
  streamsize old = width_;
  width_ = w;
  return old;
}

streamsize ios_base::precision(streamsize p) {
  streamsize old = prec_;
  prec_ = p;
  return old;
}

char ios_base::fill(char c) {
  char old = fill_;
  fill_ = c;
  return old;
}

void ios_base::clear(iostate s) {
  state_ = s;
  if (state_ & except_)
    _throw_failure();
}

void ios_base::exceptions(iostate mask) {
  except_ = mask;
  clear(state_);
}

locale ios_base::getloc() const { return loc_; }

locale ios_base::imbue(const locale& loc) {
  locale old = loc_;
  loc_ = loc;
  return old;
}

int ios_base::_localize_number_with_locale(char* out, size_t out_size,
                                           const char* input, int input_size,
                                           int character_kind) const {
  auto localize = [&]<typename Char>() {
    const auto* facet = loc_._facet<numpunct<Char>>();
    if (!facet)
      return 0;
    string grouping = facet->grouping();
    return _localize_number(out, out_size, input, input_size,
                            static_cast<char>(facet->decimal_point()),
                            static_cast<char>(facet->thousands_sep()),
                            grouping.data(), grouping.size());
  };
  switch (character_kind) {
  case 1:
    return localize.template operator()<char>();
  case 2:
    return localize.template operator()<wchar_t>();
  case 3:
    return localize.template operator()<char8_t>();
  case 4:
    return localize.template operator()<char16_t>();
  case 5:
    return localize.template operator()<char32_t>();
  }
  return 0;
}

ios_base::failure::failure(const string& message)
    : system_error(error_code(1, generic_category()), message) {}

ios_base::failure::failure(const char* message)
    : system_error(error_code(1, generic_category()), message) {}

ios_base::failure::failure(const string& message, const error_code& code)
    : system_error(code, message) {}

ios_base::failure::failure(const char* message, const error_code& code)
    : system_error(code, message) {}

void ios_base::_throw_failure_with_exceptions() {
  _PSYCHICSTD_THROW(failure("basic_ios::clear"));
}

} // namespace std
