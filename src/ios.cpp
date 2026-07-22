#include <ios>

namespace std {

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
