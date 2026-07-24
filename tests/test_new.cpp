#include "psyassert.h"
#include <new>

int main() {
  auto alignment = std::align_val_t{64};
  psyassert(static_cast<std::size_t>(alignment) == 64);

  auto* p = ::operator new(8);
  psyassert(p != nullptr);
  ::operator delete(p);
}
