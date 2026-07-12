#include <atomic>
#include <cassert>
#include <sstream>
#include <thread>

int main() {
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  std::atomic<int> ran(0);
  std::thread worker([&] { ++ran; });
  assert(worker.joinable());
  worker.join();
  assert(ran == 1);
  std::ostringstream out;
  out << std::this_thread::get_id();
  assert(!out.str().empty());
}
