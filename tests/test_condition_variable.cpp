#include <cassert>
#include <chrono>
#include <condition_variable>
#include <mutex>

// psychicstd's condition_variable is a single-threaded stub, so this only
// exercises the predicate-based API (which is well-defined even without
// real blocking): if the predicate is already true, none of the wait
// variants should ever block.
int main() {
  std::mutex m;
  std::condition_variable cv;
  bool ready = true;

  std::unique_lock<std::mutex> lock(m);
  cv.wait(lock, [&] { return ready; });

  // Note: the non-predicate wait_for/wait_until overloads are deliberately
  // not exercised here. On a real condition_variable that nobody notifies,
  // they genuinely block for the full duration and report cv_status::timeout
  // -- unlike psychicstd's single-threaded stub, which always reports
  // no_timeout immediately. Only the predicate-based overloads have
  // equivalent, well-defined semantics on both.
  bool woke =
      cv.wait_for(lock, std::chrono::milliseconds(1), [&] { return ready; });
  assert(woke);

  bool woke2 = cv.wait_until(lock, std::chrono::steady_clock::now(),
                             [&] { return ready; });
  assert(woke2);

  cv.notify_one();
  cv.notify_all();
}
