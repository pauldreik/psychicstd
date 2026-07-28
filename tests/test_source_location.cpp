#include "psyassert.h"
#include <cstring>
#include <source_location>

std::source_location
capture(std::source_location location = std::source_location::current()) {
  return location;
}

int main() {
  std::source_location empty;
  psyassert(empty.line() == 0);
  psyassert(empty.column() == 0);
  psyassert(std::strcmp(empty.file_name(), "") == 0);
  psyassert(std::strcmp(empty.function_name(), "") == 0);

  const auto expected_line = __LINE__ + 1;
  const auto location = capture();
  psyassert(location.line() == expected_line);
  psyassert(std::strstr(location.file_name(), "test_source_location.cpp"));
  psyassert(std::strlen(location.function_name()) != 0);

  static_assert(__cpp_lib_source_location == 201907L);
}
