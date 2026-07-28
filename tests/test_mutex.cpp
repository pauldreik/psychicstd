#include "psyassert.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <shared_mutex>
#include <system_error>
#include <thread>

inline int once_calls;

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
}
