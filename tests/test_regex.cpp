#include "psyassert.h"
#include <regex>

int main() {
  std::regex re("hello");
  psyassert(std::regex_match("hello", re));
}
