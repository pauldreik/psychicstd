#include "psyassert.h"
#include <string>
#include <system_error>
#include <typeindex>
#include <typeinfo>

int main() {
  psyassert(typeid(int) == typeid(int));
  const std::type_index integer(typeid(int));
  const std::type_index floating(typeid(double));
  psyassert((integer < floating) != (floating < integer));
  psyassert(std::hash<std::type_index>{}(integer) == integer.hash_code());
  psyassert(std::string(std::system_category().name()) == "system");
  psyassert(std::string(std::generic_category().name()) == "generic");
}
