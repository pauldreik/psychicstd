#include <cassert>
#include <exception>

int main() {
  std::exception e;
  assert(e.what() != nullptr);
}
