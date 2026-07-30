#include "psyassert.h"
#include <cstdint>
#include <future>
#include <vector>

namespace {

struct alignas(64) tracked_result {
  int value;
  bool* destroyed;

  tracked_result(int value, bool* destroyed)
      : value(value), destroyed(destroyed) {}
  tracked_result(tracked_result&& other) noexcept
      : value(other.value), destroyed(other.destroyed) {
    other.destroyed = nullptr;
  }
  tracked_result(const tracked_result&) = delete;
  ~tracked_result() {
    if (destroyed)
      *destroyed = true;
  }
};

} // namespace

int main() {
  std::vector<std::future<unsigned int>> futures;
  for (unsigned int i = 0; i < 4; ++i)
    futures.emplace_back(std::async(
        std::launch::async, [](unsigned int value) { return value + 1; }, i));
  unsigned int total = 0;
  for (auto& future : futures)
    total += future.get();
  psyassert(total == 10);
  psyassert(std::async(std::launch::async, [] { return 42; }).get() == 42);

  bool destroyed = false;
  {
    auto future = std::async(
        std::launch::async, [](bool* flag) { return tracked_result(42, flag); },
        &destroyed);
    auto result = future.get();
    psyassert(result.value == 42);
    psyassert(reinterpret_cast<uintptr_t>(&result) % alignof(tracked_result) ==
              0);
    psyassert(!destroyed);
  }
  psyassert(destroyed);
}
