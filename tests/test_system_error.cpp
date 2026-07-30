#include "psyassert.h"
#include <cerrno>
#include <cstring>
#include <string>
#include <system_error>

namespace custom {
enum class error { failed = 42 };
std::error_code make_error_code(error value) {
  return {static_cast<int>(value), std::generic_category()};
}
} // namespace custom

template <> struct std::is_error_code_enum<custom::error> : std::true_type {};

int main() {
  static_assert(std::is_error_condition_enum_v<std::errc>);
  psyassert(std::make_error_code(std::errc::value_too_large).value() ==
            static_cast<int>(std::errc::value_too_large));
  std::error_condition invalid = std::errc::invalid_argument;
  psyassert(invalid.value() == EINVAL);
  psyassert(invalid.category() == std::generic_category());
  psyassert(std::make_error_condition(std::errc::result_out_of_range).value() ==
            ERANGE);

  psyassert(&std::generic_category() == &std::generic_category());
  psyassert(&std::system_category() == &std::system_category());
  psyassert(&std::generic_category() != &std::system_category());
  psyassert(std::string(std::generic_category().name()) == "generic");
  psyassert(std::string(std::system_category().name()) == "system");

  auto ec = std::error_code(EDOM, std::generic_category());
  psyassert(ec.value() == EDOM);
  psyassert(ec.category() == std::generic_category());
  psyassert(static_cast<bool>(ec));
  psyassert(!std::error_code());
  std::error_code custom_error = custom::error::failed;
  psyassert(custom_error.value() == 42);
  psyassert(std::system_error(custom::error::failed).code() == custom_error);

  // Category messages come from strerror (fmt relies on this via
  // fmt::format_system_error, which compares against system_error::what()).
  std::string sys_msg = std::strerror(EDOM);
  psyassert(ec.message() == sys_msg);
  psyassert(std::system_category().message(EDOM) == sys_msg);

  // what() convention shared by libstdc++/libc++: "<what_arg>: <message>".
  psyassert(std::string(std::system_error(ec, "test").what()) ==
            "test: " + sys_msg);
  psyassert(std::string(std::system_error(EDOM, std::generic_category(), "test")
                            .what()) == "test: " + sys_msg);
  psyassert(std::string(std::system_error(ec).what()) == sys_msg);

  // system_error is catchable as runtime_error with the composed message.
  bool caught = false;
  try {
    throw std::system_error(ec, "test error");
  } catch (const std::runtime_error& e) {
    caught = std::string(e.what()) == "test error: " + sys_msg;
  } catch (...) {
  }
  psyassert(caught);

  psyassert(std::system_error(ec, "test").code() == ec);

  std::error_condition condition(EDOM, std::generic_category());
  psyassert(condition.value() == EDOM);
  psyassert(condition.category() == std::generic_category());
  psyassert(ec == condition);
}
