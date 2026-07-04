#include <cassert>
#include <stdexcept>
#include <string>

int main() {
  std::runtime_error e("test");
  assert(std::string(e.what()) == "test");
}
