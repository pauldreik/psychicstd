#include "psyassert.h"
#include <cstring>
#include <functional>

int main() {
  auto h = std::hash<const char*>{};
  psyassert(h("hello") != 0);
}
