#include "psyassert.h"
#include <stdexcept>
#include <string>

int main() {
  std::runtime_error e("test");
  psyassert(std::string(e.what()) == "test");

  std::runtime_error copied(e);
  e = std::runtime_error("changed");
  psyassert(std::string(copied.what()) == "test");

  std::logic_error logic("logic");
  std::logic_error assigned("other");
  assigned = logic;
  logic = std::logic_error("changed");
  psyassert(std::string(assigned.what()) == "logic");

  bool caught_as_logic = false;
  try {
    throw std::invalid_argument("argument");
  } catch (const std::logic_error& error) {
    caught_as_logic = std::string(error.what()) == "argument";
  }
  psyassert(caught_as_logic);

  bool caught_as_exception = false;
  try {
    throw std::overflow_error("overflow");
  } catch (const std::exception& error) {
    caught_as_exception = std::string(error.what()) == "overflow";
  }
  psyassert(caught_as_exception);
}
