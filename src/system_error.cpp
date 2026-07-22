#include <cstdio>
#include <string.h>
#include <system_error>

namespace std {

error_category::~error_category() = default;

error_condition error_category::default_error_condition(int ev) const noexcept {
  return error_condition(ev, *this);
}

bool error_category::equivalent(
    int code, const error_condition& condition) const noexcept {
  return default_error_condition(code) == condition;
}

bool error_category::equivalent(const error_code& code,
                                int condition) const noexcept {
  return *this == code.category() && code.value() == condition;
}

namespace {

// POSIX strerror_r returns int; glibc's GNU variant returns char*.
const char* strerror_result(int, const char* buffer) { return buffer; }
const char* strerror_result(const char* result, const char*) { return result; }

string errno_message(int value) {
  char buffer[128] = "";
  return string(
      strerror_result(::strerror_r(value, buffer, sizeof(buffer)), buffer));
}

class generic_category_type final : public error_category {
public:
  const char* name() const noexcept override { return "generic"; }
  string message(int value) const override { return errno_message(value); }
};

class system_category_type final : public error_category {
public:
  const char* name() const noexcept override { return "system"; }
  string message(int value) const override { return errno_message(value); }
};

string build_what(const error_code& code, const char* message) {
  string result;
  if (message && *message) {
    result = message;
    result += ": ";
  }
  result += code.message();
  return result;
}

} // namespace

const error_category& system_category() noexcept {
  static system_category_type category;
  return category;
}

const error_category& generic_category() noexcept {
  static generic_category_type category;
  return category;
}

string error_condition::message() const { return cat_->message(val_); }
string error_code::message() const { return cat_->message(val_); }

system_error::system_error(error_code code, const string& message)
    : system_error(code, message.c_str()) {}

system_error::system_error(error_code code, const char* message)
    : runtime_error(build_what(code, message)), ec_(code) {}

system_error::system_error(error_code code)
    : runtime_error(code.message()), ec_(code) {}

system_error::system_error(int value, const error_category& category,
                           const string& message)
    : system_error(error_code(value, category), message) {}

system_error::system_error(int value, const error_category& category,
                           const char* message)
    : system_error(error_code(value, category), message) {}

system_error::system_error(int value, const error_category& category)
    : system_error(error_code(value, category)) {}

system_error::~system_error() noexcept = default;

} // namespace std
