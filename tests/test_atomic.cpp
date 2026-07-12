#include <atomic>
#include <cassert>
#include <thread>

struct throwing {
  throwing() { throw 42; }
};

struct pair {
  int first;
  int second;
};

int main() {
  static_assert(std::atomic_int::is_always_lock_free ==
                (ATOMIC_INT_LOCK_FREE == 2));
  static_assert(sizeof(std::atomic_uint64_t::value_type) == 8);

  std::atomic_flag flag = ATOMIC_FLAG_INIT;
  assert(!flag.test());
  assert(!std::atomic_flag_test_and_set(&flag));
  assert(flag.test());
  std::atomic_flag_clear_explicit(&flag, std::memory_order_release);
  assert(!std::atomic_flag_test_explicit(&flag, std::memory_order_acquire));

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

  volatile std::atomic_uint bits(1);
  assert(bits.fetch_or(2) == 1);
  assert(bits.fetch_xor(1) == 3);
  assert(bits.fetch_and(2) == 2);
  assert(bits.load() == 2);

  std::atomic_init(&x, 1);
  assert(std::atomic_fetch_xor_explicit(&x, 3, std::memory_order_relaxed) == 1);
  assert(std::atomic_load(&x) == 2);
  expected = 2;
  assert(std::atomic_compare_exchange_weak(&x, &expected, 4));
  std::atomic_store_explicit(&x, 5, std::memory_order_release);
  assert(std::atomic_exchange(&x, 6) == 5);

  std::atomic<pair> record(pair{1, 2});
  pair old = record.exchange(pair{3, 4});
  assert(old.first == 1 && old.second == 2);
  pair wanted{3, 4};
  assert(record.compare_exchange_strong(wanted, pair{5, 6}));
  assert(record.load().second == 6);

  int values[4] = {};
  std::atomic<int*> pointer(values);
  assert(pointer.fetch_add(2) == values);
  assert(pointer.load() == values + 2);
  assert(--pointer == values + 1);

  std::atomic<double> number(1.5);
  assert(number.fetch_add(0.5) == 1.5);
  assert((number -= 0.25) == 1.75);

  alignas(std::atomic_ref<int>::required_alignment) int referenced = 1;
  std::atomic_ref ref(referenced);
  assert(ref.fetch_add(2) == 1);
  assert(referenced == 3);
  expected = 3;
  assert(ref.compare_exchange_strong(expected, 4));
  assert(referenced == 4);

  std::atomic<int> state(0);
  int observed = 0;
  std::thread waiter([&] {
    state.wait(0);
    observed = state.load();
  });
  state.store(1);
  state.notify_one();
  waiter.join();
  assert(observed == 1);

  state.store(2);
  std::atomic_wait(&state, 1);
  flag.test_and_set();
  flag.notify_all();
  flag.wait(false);
}
