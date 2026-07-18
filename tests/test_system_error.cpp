#include "psyassert.h"
#include <cerrno>
#include <cstring>
#include <string>
#include <system_error>

int main() {
  auto ec = std::error_code(EDOM, std::generic_category());
  psyassert(ec.value() == EDOM);
  psyassert(ec.category() == std::generic_category());
  psyassert(static_cast<bool>(ec));
  psyassert(!std::error_code());

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
}
