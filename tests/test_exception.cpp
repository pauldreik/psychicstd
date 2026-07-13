#include "psyassert.h"
#include <exception>

int main() {
  std::exception e;
  psyassert(e.what() != nullptr);
}
