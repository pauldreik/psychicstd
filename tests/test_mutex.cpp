#include "psyassert.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

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
  std::mutex m;
  m.lock();
  psyassert(!try_from_another_thread(m));
  m.unlock();
  psyassert(try_from_another_thread(m));

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
