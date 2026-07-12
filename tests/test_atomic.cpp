#include <atomic>
#include <cassert>
#include <thread>

struct throwing {
  throwing() { throw 42; }
};

int main() {
  try {
    std::atomic<throwing> value;
    (void)value;
    assert(false);
  } catch (int value) {
    assert(value == 42);
  }

  std::atomic<int> x{42};
  assert(x.load() == 42);

  x.store(0, std::memory_order_relaxed);
  auto increment = [&] {
    for (int i = 0; i < 10000; ++i)
      ++x;
  };
  std::thread first(increment);
  std::thread second(increment);
  first.join();
  second.join();
  assert(x.load() == 20000);

  int expected = 20000;
  assert(x.compare_exchange_strong(expected, 7));
  assert(x.exchange(9) == 7);
  assert(x.fetch_add(2) == 9);
  assert(x.load() == 11);
}
