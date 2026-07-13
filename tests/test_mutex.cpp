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
}
