#include "psyassert.h"
#include <string>
#include <system_error>
#include <typeinfo>

int main() {
  psyassert(typeid(int) == typeid(int));
  psyassert(std::string(std::system_category().name()) == "system");
  psyassert(std::string(std::generic_category().name()) == "generic");
}
