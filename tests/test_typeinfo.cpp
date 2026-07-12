#include <cassert>
#include <string>
#include <system_error>
#include <typeinfo>

int main() {
  assert(typeid(int) == typeid(int));
  assert(std::string(std::system_category().name()) == "system");
  assert(std::string(std::generic_category().name()) == "generic");
}
