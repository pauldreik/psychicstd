#include "psyassert.h"
#include <atomic>
#include <thread>

struct throwing {
  throwing() { throw 42; }
};

struct pair {
  int first;
  int second;
};

struct alignas(16) wide_value {
  int first;
  int second;
  int third;
  int fourth;
};

int main() {
  static_assert(std::atomic_int::is_always_lock_free ==
                (ATOMIC_INT_LOCK_FREE == 2));
  static_assert(sizeof(std::atomic_uint64_t::value_type) == 8);

  std::atomic_flag flag = ATOMIC_FLAG_INIT;
  psyassert(!flag.test());
  psyassert(!std::atomic_flag_test_and_set(&flag));
  psyassert(flag.test());
  std::atomic_flag_clear_explicit(&flag, std::memory_order_release);
  psyassert(!std::atomic_flag_test_explicit(&flag, std::memory_order_acquire));

  try {
    std::atomic<throwing> value;
    (void)value;
    psyassert(false);
  } catch (int value) {
    psyassert(value == 42);
  }

  std::atomic<int> x{42};
  psyassert(x.load() == 42);

  x.store(0, std::memory_order_relaxed);
  auto increment = [&] {
    for (int i = 0; i < 10000; ++i)
      ++x;
  };
  std::thread first(increment);
  std::thread second(increment);
  first.join();
  second.join();
  psyassert(x.load() == 20000);

  int expected = 20000;
  psyassert(x.compare_exchange_strong(expected, 7));
  psyassert(x.exchange(9) == 7);
  psyassert(x.fetch_add(2) == 9);
  psyassert(x.load() == 11);

  volatile std::atomic_uint bits(1);
  psyassert(bits.fetch_or(2) == 1);
  psyassert(bits.fetch_xor(1) == 3);
  psyassert(bits.fetch_and(2) == 2);
  psyassert(bits.load() == 2);

  std::atomic_init(&x, 1);
  psyassert(std::atomic_fetch_xor_explicit(&x, 3, std::memory_order_relaxed) ==
            1);
  psyassert(std::atomic_load(&x) == 2);
  expected = 2;
  psyassert(std::atomic_compare_exchange_weak(&x, &expected, 4));
  std::atomic_store_explicit(&x, 5, std::memory_order_release);
  psyassert(std::atomic_exchange(&x, 6) == 5);

  std::atomic<pair> record(pair{1, 2});
  pair old = record.exchange(pair{3, 4});
  psyassert(old.first == 1 && old.second == 2);
  pair wanted{3, 4};
  psyassert(record.compare_exchange_strong(wanted, pair{5, 6}));
  psyassert(record.load().second == 6);

  std::atomic<wide_value> wide_record(wide_value{1, 2, 3, 4});
  wide_value wide_wanted{1, 2, 3, 4};
  psyassert(
      wide_record.compare_exchange_strong(wide_wanted, wide_value{5, 6, 7, 8}));
  psyassert(wide_record.load().fourth == 8);

  int values[4] = {};
  std::atomic<int*> pointer(values);
  psyassert(pointer.fetch_add(2) == values);
  psyassert(pointer.load() == values + 2);
  psyassert(--pointer == values + 1);

  std::atomic<double> number(1.5);
  psyassert(number.fetch_add(0.5) == 1.5);
  psyassert((number -= 0.25) == 1.75);

  alignas(std::atomic_ref<int>::required_alignment) int referenced = 1;
  std::atomic_ref ref(referenced);
  psyassert(ref.fetch_add(2) == 1);
  psyassert(referenced == 3);
  expected = 3;
  psyassert(ref.compare_exchange_strong(expected, 4));
  psyassert(referenced == 4);

  std::atomic<int> state(0);
  int observed = 0;
  std::thread waiter([&] {
    state.wait(0);
    observed = state.load();
  });
  state.store(1);
  state.notify_one();
  waiter.join();
  psyassert(observed == 1);

  state.store(2);
  std::atomic_wait(&state, 1);
  flag.test_and_set();
  flag.notify_all();
  flag.wait(false);
}
