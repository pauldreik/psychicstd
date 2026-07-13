#include <atomic>
#include <cassert>
#include <sstream>
#include <stop_token>
#include <thread>

int main() {
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  std::atomic<int> ran(0);
  std::thread thread_worker([&] { ++ran; });
  assert(thread_worker.joinable());
  thread_worker.join();
  assert(ran == 1);
  std::ostringstream out;
  out << std::this_thread::get_id();
  assert(!out.str().empty());

  std::stop_source source;
  std::stop_token token = source.get_token();
  assert(source.stop_possible());
  assert(token.stop_possible());
  assert(!token.stop_requested());

  std::atomic<int> callback_runs(0);
  {
    std::stop_callback callback(token, [&] { ++callback_runs; });
    bool requested = source.request_stop();
    assert(requested);
    assert(callback_runs == 1);
  }
  assert(token.stop_requested());

  std::atomic<bool> started(false);
  std::atomic<bool> stopped(false);
  std::jthread jworker([&](std::stop_token stop) {
    started.store(true, std::memory_order_relaxed);
    while (!stop.stop_requested())
      std::this_thread::yield();
    stopped.store(true, std::memory_order_relaxed);
  });
  while (!started.load(std::memory_order_relaxed))
    std::this_thread::yield();
  assert(jworker.joinable());
  assert(jworker.get_stop_token().stop_possible());
  bool requested = jworker.request_stop();
  assert(requested);
  jworker.join();
  assert(stopped.load(std::memory_order_relaxed));
  assert(!jworker.joinable());

  std::atomic<int> plain_runs(0);
  std::jthread plain([&] { ++plain_runs; });
  plain.join();
  assert(plain_runs == 1);
}
