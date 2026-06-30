#include <cassert>
#include <mutex>

int main() {
  std::mutex m;
  m.lock();
  m.unlock();
}
