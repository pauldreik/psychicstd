#include <any>
#include <memory>
#include <optional>
#include <stdexcept>
#include <variant>

namespace std {

namespace __stdexcept {

char* __msg::__dup(const char* s) {
  if (!s)
    return nullptr;
  auto n = __builtin_strlen(s) + 1;
  char* result = new char[n];
  __builtin_memcpy(result, s, n);
  return result;
}

__msg::__msg(const char* s) : p_(__dup(s)) {}
__msg::__msg(const __msg& other) : p_(__dup(other.p_)) {}
__msg::__msg(__msg&& other) noexcept : p_(other.p_) { other.p_ = nullptr; }

__msg& __msg::operator=(const __msg& other) {
  if (this != &other) {
    char* replacement = __dup(other.p_);
    delete[] p_;
    p_ = replacement;
  }
  return *this;
}

__msg& __msg::operator=(__msg&& other) noexcept {
  if (this != &other) {
    delete[] p_;
    p_ = other.p_;
    other.p_ = nullptr;
  }
  return *this;
}

__msg::~__msg() { delete[] p_; }

const char* __msg::c_str() const noexcept { return p_ ? p_ : ""; }

} // namespace __stdexcept

logic_error::logic_error(const char* msg) : msg_(msg) {}
logic_error::logic_error(const logic_error&) = default;
logic_error& logic_error::operator=(const logic_error&) = default;
logic_error::~logic_error() noexcept = default;
const char* logic_error::what() const noexcept { return msg_.c_str(); }

domain_error::~domain_error() noexcept = default;
invalid_argument::~invalid_argument() noexcept = default;
length_error::~length_error() noexcept = default;
out_of_range::~out_of_range() noexcept = default;

runtime_error::runtime_error(const char* msg) : msg_(msg) {}
runtime_error::runtime_error(const runtime_error&) = default;
runtime_error& runtime_error::operator=(const runtime_error&) = default;
runtime_error::~runtime_error() noexcept = default;
const char* runtime_error::what() const noexcept { return msg_.c_str(); }

range_error::~range_error() noexcept = default;
overflow_error::~overflow_error() noexcept = default;
underflow_error::~underflow_error() noexcept = default;

void __throw_out_of_range(const char* message) {
  _PSYCHICSTD_THROW(out_of_range(message));
}
void __throw_invalid_argument(const char* message) {
  _PSYCHICSTD_THROW(invalid_argument(message));
}
void __throw_runtime_error(const char* message) {
  _PSYCHICSTD_THROW(runtime_error(message));
}

void __throw_bad_optional_access() { _PSYCHICSTD_THROW(bad_optional_access()); }
void __throw_bad_variant_access() { _PSYCHICSTD_THROW(bad_variant_access()); }
void __throw_bad_any_cast() { _PSYCHICSTD_THROW(bad_any_cast()); }
void __throw_bad_weak_ptr() { _PSYCHICSTD_THROW(bad_weak_ptr()); }

} // namespace std
