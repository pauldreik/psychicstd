#include "psyassert.h"
#include <stdexcept>
#include <string>

int main() {
  std::runtime_error e("test");
  psyassert(std::string(e.what()) == "test");
}
