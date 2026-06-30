#include <cassert>
#include <stdexcept>

int main() {
  std::runtime_error e("test");
  assert(std::string(e.what()) == "test");
}
