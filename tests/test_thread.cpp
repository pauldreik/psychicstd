#include "psyassert.h"
#include <atomic>
#include <cstdlib>
#include <exception>
#include <sstream>
#include <stop_token>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <utility>

struct worker {
  std::atomic<int>* runs;
  void run(int count) { runs->fetch_add(count, std::memory_order_relaxed); }
};

void expect_terminate(void (*operation)()) {
  pid_t child = fork();
  psyassert(child >= 0);
  if (child == 0) {
    std::set_terminate([] { std::_Exit(0); });
    operation();
    std::_Exit(1);
  }
  int status = 0;
  psyassert(waitpid(child, &status, 0) == child);
  psyassert(WIFEXITED(status));
  psyassert(WEXITSTATUS(status) == 0);
}

int main() {
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  std::atomic<int> ran(0);
  std::thread thread_worker([&] { ++ran; });
  psyassert(thread_worker.joinable());
  thread_worker.join();
  psyassert(ran == 1);
  worker member_worker{&ran};
  std::thread member_thread(&worker::run, &member_worker, 2);
  member_thread.join();
  psyassert(ran == 3);
  std::ostringstream out;
  out << std::this_thread::get_id();
  psyassert(!out.str().empty());

  std::stop_source source;
  std::stop_token token = source.get_token();
  psyassert(source.stop_possible());
  psyassert(token.stop_possible());
  psyassert(!token.stop_requested());

  std::atomic<int> callback_runs(0);
  {
    std::stop_callback callback(token, [&] { ++callback_runs; });
    bool requested = source.request_stop();
    psyassert(requested);
    psyassert(callback_runs == 1);
  }
  psyassert(token.stop_requested());

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
  psyassert(jworker.joinable());
  psyassert(jworker.get_stop_token().stop_possible());
  bool requested = jworker.request_stop();
  psyassert(requested);
  jworker.join();
  psyassert(stopped.load(std::memory_order_relaxed));
  psyassert(!jworker.joinable());

  std::atomic<int> plain_runs(0);
  std::jthread plain([&] { ++plain_runs; });
  plain.join();
  psyassert(plain_runs == 1);

  expect_terminate(+[] { std::thread joinable([] {}); });
  expect_terminate(+[] {
    std::thread joinable([] {});
    std::thread empty;
    joinable = std::move(empty);
  });
}
