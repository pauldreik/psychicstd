#include <ctime>
#include <cwchar>
#include <locale>
#include <stdexcept>
#include <typeinfo>

namespace std {

void __throw_bad_cast() { _PSYCHICSTD_THROW(bad_cast()); }

struct locale::_node {
  long refs = 1;
  facet* value;
  _node* next;

  _node(facet* f, _node* n) : value(f), next(n) {
    if (next)
      ++next->refs;
  }
};

struct locale::_rep {
  long refs = 1;
  _node* head = nullptr;
  char* name = nullptr;

  _rep(facet* f, _node* inherited) : head(new _node(f, inherited)) {}
  explicit _rep(const char* locale_name);
  ~_rep() {
    _release_node(head);
    delete[] name;
  }

private:
  static void _release_node(_node* node) noexcept {
    if (node && --node->refs == 0) {
      _node* next = node->next;
      delete node->value;
      delete node;
      _release_node(next);
    }
  }
};

namespace {

bool same_name(const char* left, const char* right) noexcept {
  while (*left && *left == *right) {
    ++left;
    ++right;
  }
  return *left == *right;
}

}

locale::locale() noexcept = default;

locale::locale(const char* name) : rep_(_make_named(name)) {}

locale::locale(const locale& other) noexcept : rep_(other.rep_) {
  if (rep_)
    ++rep_->refs;
}

locale& locale::operator=(const locale& other) noexcept {
  if (rep_ != other.rep_) {
    if (other.rep_)
      ++other.rep_->refs;
    _release();
    rep_ = other.rep_;
  }
  return *this;
}

locale::~locale() { _release(); }

const locale& locale::classic() {
  static locale classic_locale;
  return classic_locale;
}

locale locale::global(const locale& loc) {
  locale& current = _global();
  locale previous = current;
  current = loc;
  return previous;
}

bool locale::operator==(const locale& other) const noexcept {
  if (rep_ == other.rep_)
    return true;
  return rep_ && other.rep_ && rep_->name && other.rep_->name &&
         same_name(rep_->name, other.rep_->name);
}

bool locale::operator!=(const locale& other) const noexcept {
  return !(*this == other);
}

bool locale::_has_facets() const noexcept { return rep_ && rep_->head; }

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

locale::_rep* locale::_with_facet(_rep* other, facet* f) {
  return new _rep(f, other ? other->head : nullptr);
}

locale::_node* locale::_first_facet(_rep* rep) noexcept {
  return rep ? rep->head : nullptr;
}

locale::_node* locale::_next_facet(_node* node) noexcept { return node->next; }

locale::facet* locale::_facet_value(_node* node) noexcept {
  return node->value;
}

void locale::_release() noexcept {
  if (rep_ && --rep_->refs == 0)
    delete rep_;
}

locale& locale::_global() {
  static locale global_locale;
  return global_locale;
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

}
}
