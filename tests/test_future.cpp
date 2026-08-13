#include "psyassert.h"
#include <chrono>
#include <cstdint>
#include <future>
#include <stdexcept>
#include <thread>
#include <vector>

namespace {

struct alignas(64) tracked_result {
  int value;
  bool* destroyed;

  tracked_result(int value, bool* destroyed)
      : value(value), destroyed(destroyed) {}
  tracked_result(tracked_result&& other) noexcept
      : value(other.value), destroyed(other.destroyed) {
    other.destroyed = nullptr;
  }
  tracked_result(const tracked_result&) = delete;
  ~tracked_result() {
    if (destroyed)
      *destroyed = true;
  }
};

struct throwing_move_result {
  bool* stored_destroyed;
  int generation = 0;

  explicit throwing_move_result(bool* stored_destroyed)
      : stored_destroyed(stored_destroyed) {}
  throwing_move_result(throwing_move_result&& other)
      : stored_destroyed(other.stored_destroyed),
        generation(other.generation + 1) {
    if (other.generation == 1)
      throw std::runtime_error("move failed");
  }
  throwing_move_result(const throwing_move_result&) = delete;
  ~throwing_move_result() {
    if (generation == 1)
      *stored_destroyed = true;
  }
};

} // namespace

template <typename F> static void expect_exception(F&& function) {
  bool threw = false;
  try {
    static_cast<F&&>(function)();
  } catch (const std::exception&) {
    threw = true;
  }
  psyassert(threw);
}

static void test_async_values() {
  std::vector<std::future<unsigned int>> futures;
  for (unsigned int i = 0; i < 4; ++i)
    futures.emplace_back(std::async(
        std::launch::async, [](unsigned int value) { return value + 1; }, i));
  unsigned int total = 0;
  for (auto& future : futures)
    total += future.get();
  psyassert(total == 10);
  psyassert(std::async(std::launch::async, [] { return 42; }).get() == 42);
}

static void test_async_void() {
  std::async(std::launch::async, [] {}).get();
}

static void test_async_exception() {
  expect_exception([] {
    std::async(std::launch::async, []() -> int {
      throw std::runtime_error("async failed");
    }).get();
  });
}

static void test_overaligned_result_lifetime() {
  bool destroyed = false;
  {
    auto future = std::async(
        std::launch::async, [](bool* flag) { return tracked_result(42, flag); },
        &destroyed);
    auto result = future.get();
    psyassert(result.value == 42);
    psyassert(reinterpret_cast<uintptr_t>(&result) % alignof(tracked_result) ==
              0);
    psyassert(!destroyed);
  }
  psyassert(destroyed);
}

static void test_promise_values() {
  std::promise<int> int_promise;
  std::future<int> int_future = int_promise.get_future();
  const int value = 42;
  int_promise.set_value(value);
  psyassert(int_future.get() == 42);

  std::promise<void> void_promise;
  std::future<void> void_future = void_promise.get_future();
  void_promise.set_value();
  void_future.get(); // must not throw
}

static void test_wait_for() {
  std::promise<int> wait_promise;
  std::future<int> wait_future = wait_promise.get_future();
  psyassert(wait_future.wait_for(std::chrono::milliseconds(10)) ==
            std::future_status::timeout);
  wait_promise.set_value(7);
  psyassert(wait_future.wait_for(std::chrono::seconds(1)) ==
            std::future_status::ready);
  psyassert(wait_future.get() == 7);
}

static void test_promise_exception() {
  std::promise<int> exception_promise;
  std::future<int> exception_future = exception_promise.get_future();
  exception_promise.set_exception(
      std::make_exception_ptr(std::runtime_error("boom")));
  bool rethrew = false;
  try {
    (void)exception_future.get();
  } catch (const std::runtime_error& e) {
    rethrew = true;
    psyassert(__builtin_strcmp(e.what(), "boom") == 0);
  }
  psyassert(rethrew);
}

static void test_throwing_result_move() {
  bool stored_destroyed = false;
  {
    std::promise<throwing_move_result> promise;
    auto future = promise.get_future();
    promise.set_value(throwing_move_result(&stored_destroyed));
    expect_exception([&] { (void)future.get(); });
    psyassert(!future.valid());
  }
  psyassert(stored_destroyed);
}

static void test_broken_promise() {
  std::future<int> broken_future;
  {
    std::promise<int> broken_promise;
    broken_future = broken_promise.get_future();
  }
  psyassert(broken_future.wait_for(std::chrono::seconds(1)) ==
            std::future_status::ready);
  expect_exception([&] { (void)broken_future.get(); });
}

static void test_promise_state_errors() {
  std::promise<int> promise;
  auto future = promise.get_future();
  expect_exception([&] { (void)promise.get_future(); });
  promise.set_value(1);
  expect_exception([&] { promise.set_value(2); });
  psyassert(future.get() == 1);
  psyassert(!future.valid());

  std::promise<int> moved_from;
  std::promise<int> moved_to(static_cast<std::promise<int>&&>(moved_from));
  expect_exception([&] { moved_from.set_value(3); });
}

static void test_packaged_task_move_only_callable() {
  // packaged_task constructed from a lambda that captures a move-only
  // object (a std::promise moved into the capture list) -- packaged_task's
  // target must only be required to be MoveConstructible, not
  // CopyConstructible. A first implementation attempt backed by
  // std::function would reject this (function<>::clone() requires a
  // copyable target).
  std::promise<int> inner_promise;
  std::future<int> inner_future = inner_promise.get_future();
  std::packaged_task<int()> movable_task(
      [inner = static_cast<std::promise<int>&&>(inner_promise)]() mutable {
        inner.set_value(123);
        return 7;
      });
  std::future<int> task_future = movable_task.get_future();
  movable_task();
  psyassert(task_future.get() == 7);
  psyassert(inner_future.get() == 123);
}

static void test_packaged_task_exception() {
  std::packaged_task<int()> throwing_task(
      []() -> int { throw std::runtime_error("task failed"); });
  std::future<int> throwing_future = throwing_task.get_future();
  throwing_task();
  bool task_rethrew = false;
  try {
    (void)throwing_future.get();
  } catch (const std::runtime_error&) {
    task_rethrew = true;
  }
  psyassert(task_rethrew);
}

static void test_packaged_task_state_errors() {
  std::packaged_task<int()> empty_task;
  expect_exception([&] { (void)empty_task.get_future(); });
  expect_exception([&] { empty_task(); });

  int calls = 0;
  std::packaged_task<int()> task([&] {
    ++calls;
    return 4;
  });
  auto future = task.get_future();
  expect_exception([&] { (void)task.get_future(); });
  task();
  expect_exception([&] { task(); });
  psyassert(calls == 1);
  psyassert(future.get() == 4);
}

int main() {
  test_async_values();
  test_async_void();
  test_async_exception();
  test_overaligned_result_lifetime();
  test_promise_values();
  test_wait_for();
  test_promise_exception();
#if !defined(__APPLE__) || defined(PSYCHICSTD_TEST_PSYCHICSTD)
  // Apple libc++ crashes when the move performed by future::get() throws.
  test_throwing_result_move();
#endif
  test_broken_promise();
  test_promise_state_errors();
  test_packaged_task_move_only_callable();
  test_packaged_task_exception();
  test_packaged_task_state_errors();
}
