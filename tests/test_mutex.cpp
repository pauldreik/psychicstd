#include "psyassert.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <shared_mutex>
#include <system_error>
#include <thread>

inline int once_calls;

struct address_hiding_mutex {
  address_hiding_mutex* operator&() = delete;
  void lock() noexcept {}
  bool try_lock() noexcept { return true; }
  void unlock() noexcept {}
  void lock_shared() noexcept {}
  bool try_lock_shared() noexcept { return true; }
  void unlock_shared() noexcept {}
};

template <typename T> void self_move_assign(T& value) {
  value = static_cast<T&&>(value);
}

template <typename Mutex> bool try_from_another_thread(Mutex& mutex) {
  std::atomic<bool> acquired(false);
  std::thread thread([&] {
    acquired = mutex.try_lock();
    if (acquired)
      mutex.unlock();
  });
  thread.join();
  return acquired;
}

static void test_condition_variable_any_destruction() {
  auto* condition = new std::condition_variable_any;
  std::mutex mutex;
  bool waiter_ready = false;
  bool destroyer_ready = false;

  std::thread waiter([&] {
    mutex.lock();
    waiter_ready = true;
    condition->notify_one();
    while (!destroyer_ready)
      condition->wait(mutex);
    mutex.unlock();
  });

  mutex.lock();
  while (!waiter_ready)
    condition->wait(mutex);
  mutex.unlock();

  std::thread destroyer([&] {
    mutex.lock();
    destroyer_ready = true;
    condition->notify_one();
    delete condition;
    mutex.unlock();
  });

  destroyer.join();
  waiter.join();
}

int main() {
  std::once_flag once;
  std::thread once_first([&] { std::call_once(once, [] { ++once_calls; }); });
  std::thread once_second([&] { std::call_once(once, [] { ++once_calls; }); });
  once_first.join();
  once_second.join();
  psyassert(once_calls == 1);

  std::once_flag retry;
  bool first_attempt = true;
  try {
    std::call_once(retry, [&] {
      first_attempt = false;
      throw 1;
    });
  } catch (int) {
  }
  std::call_once(retry, [&] { ++once_calls; });
  psyassert(!first_attempt);
  psyassert(once_calls == 2);

  std::mutex m;
  m.lock();
  psyassert(!try_from_another_thread(m));
  m.unlock();
  psyassert(try_from_another_thread(m));

  std::mutex first;
  std::mutex second;
  std::unique_lock first_lock(first, std::defer_lock);
  std::unique_lock second_lock(second, std::defer_lock);
  std::lock(first_lock, second_lock);
  psyassert(first_lock.owns_lock());
  psyassert(second_lock.owns_lock());
  first_lock.unlock();
  second_lock.unlock();
  first_lock.swap(second_lock);
  psyassert(first_lock.mutex() == &second);
  psyassert(second_lock.mutex() == &first);

  std::unique_lock released_lock(m);
  psyassert(released_lock.release() == &m);
  psyassert(!released_lock.owns_lock());
  psyassert(released_lock.mutex() == nullptr);
  psyassert(!try_from_another_thread(m));
  m.unlock();
  psyassert(try_from_another_thread(m));

  std::unique_lock unique_deferred(m, std::defer_lock);
  unique_deferred.lock();
  bool unique_relock_threw = false;
  try {
    (void)unique_deferred.try_lock();
  } catch (const std::system_error& error) {
    unique_relock_threw =
        error.code() == std::errc::resource_deadlock_would_occur;
  }
  psyassert(unique_relock_threw);
  unique_deferred.unlock();
  bool unique_reunlock_threw = false;
  try {
    unique_deferred.unlock();
  } catch (const std::system_error& error) {
    unique_reunlock_threw = error.code() == std::errc::operation_not_permitted;
  }
  psyassert(unique_reunlock_threw);

  std::unique_lock<std::mutex> empty_unique;
  bool empty_unique_threw = false;
  try {
    empty_unique.lock();
  } catch (const std::system_error& error) {
    empty_unique_threw = error.code() == std::errc::operation_not_permitted;
  }
  psyassert(empty_unique_threw);

// The standard requires these locks to preserve state on self-move; libstdc++
// does not.
#ifdef PSYCHICSTD_TEST_PSYCHICSTD
  std::unique_lock unique_self_move(m, std::defer_lock);
  self_move_assign(unique_self_move);
  psyassert(unique_self_move.mutex() == &m);
  psyassert(!unique_self_move.owns_lock());
#endif

  address_hiding_mutex hidden;
  std::unique_lock hidden_unique(hidden, std::defer_lock);
  psyassert(hidden_unique.mutex() == __builtin_addressof(hidden));
  std::shared_lock hidden_shared(hidden, std::defer_lock);
  psyassert(hidden_shared.mutex() == __builtin_addressof(hidden));

  std::shared_mutex shared;
  std::shared_lock first_reader(shared);
  std::shared_lock second_reader(shared, std::try_to_lock);
  psyassert(first_reader.owns_lock());
  psyassert(second_reader.owns_lock());
  psyassert(!shared.try_lock());
  second_reader.unlock();
  first_reader.unlock();
  psyassert(shared.try_lock());
  shared.unlock();

  std::shared_lock<std::shared_mutex> deferred(shared, std::defer_lock);
  deferred.lock();
  bool relock_threw = false;
  try {
    (void)deferred.try_lock();
  } catch (const std::system_error& error) {
    relock_threw = error.code() == std::errc::resource_deadlock_would_occur;
  }
  psyassert(relock_threw);
  deferred.unlock();
  bool reunlock_threw = false;
  try {
    deferred.unlock();
  } catch (const std::system_error& error) {
    reunlock_threw = error.code() == std::errc::operation_not_permitted;
  }
  psyassert(reunlock_threw);

  std::shared_lock<std::shared_mutex> empty;
  bool empty_threw = false;
  try {
    empty.lock();
  } catch (const std::system_error& error) {
    empty_threw = error.code() == std::errc::operation_not_permitted;
  }
  psyassert(empty_threw);

// The standard requires these locks to preserve state on self-move; libstdc++
// does not.
#ifdef PSYCHICSTD_TEST_PSYCHICSTD
  std::shared_lock shared_self_move(shared, std::defer_lock);
  self_move_assign(shared_self_move);
  psyassert(shared_self_move.mutex() == &shared);
  psyassert(!shared_self_move.owns_lock());
#endif

  std::shared_timed_mutex shared_timed;
  shared_timed.lock_shared();
  psyassert(!shared_timed.try_lock());
  shared_timed.unlock_shared();

  std::recursive_mutex recursive;
  psyassert(recursive.try_lock());
  psyassert(recursive.try_lock());
  psyassert(!try_from_another_thread(recursive));
  recursive.unlock();
  psyassert(!try_from_another_thread(recursive));
  recursive.unlock();
  psyassert(try_from_another_thread(recursive));

  std::condition_variable condition;
  bool ready = false;
  bool observed = false;
  std::thread waiter([&] {
    std::unique_lock lock(m);
    condition.wait(lock, [&] { return ready; });
    observed = true;
  });
  {
    std::lock_guard lock(m);
    ready = true;
  }
  condition.notify_one();
  waiter.join();
  psyassert(observed);

  std::unique_lock timeout_lock(m);
  auto start = std::chrono::steady_clock::now();
  psyassert(condition.wait_for(timeout_lock, std::chrono::milliseconds(10)) ==
            std::cv_status::timeout);
  psyassert(std::chrono::steady_clock::now() - start >=
            std::chrono::milliseconds(10));
  psyassert(!condition.wait_until(
      timeout_lock, std::chrono::steady_clock::now(), [] { return false; }));
  timeout_lock.unlock();

  ready = false;
  observed = false;
  std::thread timed_waiter([&] {
    std::unique_lock lock(m);
    observed = condition.wait_for(lock, std::chrono::seconds(1),
                                  [&] { return ready; });
  });
  {
    std::lock_guard lock(m);
    ready = true;
  }
  condition.notify_one();
  timed_waiter.join();
  psyassert(observed);

  std::condition_variable_any any_condition;
  std::recursive_mutex any_mutex;
  bool any_ready = false;
  std::thread any_waiter([&] {
    std::unique_lock lock(any_mutex);
    any_condition.wait(lock, [&] { return any_ready; });
  });
  {
    std::lock_guard lock(any_mutex);
    any_ready = true;
  }
  any_condition.notify_one();
  any_waiter.join();

  test_condition_variable_any_destruction();
}
