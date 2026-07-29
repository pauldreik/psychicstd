#include "psyassert.h"
#include <new>

int main() {
  auto* p = ::operator new(8);
  psyassert(p != nullptr);
  ::operator delete(p);

  auto* nothrow_p = ::operator new(8, std::nothrow);
  psyassert(nothrow_p != nullptr);
  ::operator delete(nothrow_p, std::nothrow);
}
