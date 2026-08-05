#include "psyassert.h"
#include <string>
#include <system_error>
#include <typeindex>
#include <typeinfo>

struct synthetic_type_info : std::type_info {
  explicit synthetic_type_info(const char* name, bool unique = false)
      : std::type_info(encode(name, unique)) {}

private:
  static const char* encode(const char* name, bool unique) {
#if defined(__APPLE__) && defined(__aarch64__)
    // Apple marks non-unique RTTI names in bit 63 of the pointer.
    size_t bits = reinterpret_cast<size_t>(name);
    if (!unique)
      bits |= size_t(1) << 63;
    return reinterpret_cast<const char*>(bits);
#else
    (void)unique;
    return name;
#endif
  }
};

int main() {
  psyassert(typeid(int) == typeid(int));
  const std::type_index integer(typeid(int));
  const std::type_index floating(typeid(double));
  psyassert((integer < floating) != (floating < integer));
  psyassert(std::hash<std::type_index>{}(integer) == integer.hash_code());
  char first_name[] = "synthetic";
  char second_name[] = "synthetic";
  const synthetic_type_info first(first_name);
  const synthetic_type_info second(second_name);
  psyassert(first == second);
  psyassert(first.hash_code() == second.hash_code());
  char first_local_name[] = "*local";
  char second_local_name[] = "*local";
  const synthetic_type_info first_local(first_local_name, true);
  const synthetic_type_info second_local(second_local_name, true);
  psyassert(first_local != second_local);
  psyassert(std::string(std::system_category().name()) == "system");
  psyassert(std::string(std::generic_category().name()) == "generic");
}
