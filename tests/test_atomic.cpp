#include <atomic>
#include <cassert>

int main() {
  std::atomic<int> x{42};
  assert(x.load() == 42);
}
