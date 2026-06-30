#include <cassert>
#include <new>

int main() {
  auto* p = ::operator new(8);
  assert(p != nullptr);
  ::operator delete(p);
}
