#include "psyassert.h"
#include <new>

int main() {
  auto* p = ::operator new(8);
  psyassert(p != nullptr);
  ::operator delete(p);
}
