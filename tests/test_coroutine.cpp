#include "psyassert.h"
#include <coroutine>
#include <type_traits>

struct resumable {
  struct promise_type;
  using handle_type = std::coroutine_handle<promise_type>;

  struct promise_type {
    int value = 0;

    resumable get_return_object() {
      return resumable(handle_type::from_promise(*this));
    }
    std::suspend_always initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() noexcept {}
    void unhandled_exception() {}
  };

  explicit resumable(handle_type handle) : handle(handle) {}
  resumable(const resumable&) = delete;
  ~resumable() { handle.destroy(); }

  handle_type handle;
};

resumable make_resumable() {
  co_await std::suspend_always{};
  co_return;
}

int main() {
  static_assert(__cpp_lib_coroutine == 201902L);
  static_assert(std::is_same_v<std::coroutine_traits<resumable>::promise_type,
                               resumable::promise_type>);

  std::coroutine_handle<> empty;
  psyassert(!empty);
  psyassert(empty == nullptr);

  auto coroutine = make_resumable();
  psyassert(coroutine.handle);
  psyassert(!coroutine.handle.done());
  coroutine.handle.promise().value = 7;
  psyassert(coroutine.handle.promise().value == 7);

  std::coroutine_handle<> erased = coroutine.handle;
  psyassert(erased.address() == coroutine.handle.address());
  erased.resume();
  psyassert(!erased.done());
  coroutine.handle();
  psyassert(coroutine.handle.done());

  const auto noop = std::noop_coroutine();
  psyassert(noop);
  psyassert(!noop.done());
  psyassert(noop.address() != nullptr);
  noop.resume();
  noop.destroy();

  static_assert(std::suspend_never{}.await_ready());
  static_assert(!std::suspend_always{}.await_ready());
}
