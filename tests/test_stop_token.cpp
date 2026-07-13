#include <atomic>
#include <cassert>
#include <stop_token>
#include <utility>

int main() {
  std::stop_source source;
  std::stop_token token = source.get_token();
  assert(source.stop_possible());
  assert(token.stop_possible());
  assert(!source.stop_requested());
  assert(!token.stop_requested());

  std::stop_token copied = token;
  std::stop_token moved = std::move(copied);
  assert(moved.stop_possible());
  assert(copied.stop_possible() == false);

  std::atomic<int> callback_runs(0);
  {
    std::stop_callback callback(token, [&] { ++callback_runs; });
    bool requested = source.request_stop();
    assert(requested);
    assert(callback_runs == 1);
  }
  assert(source.stop_requested());
  assert(token.stop_requested());
  assert(moved.stop_requested());

  std::atomic<int> late_runs(0);
  std::stop_callback late(token, [&] { ++late_runs; });
  assert(late_runs == 1);

  std::stop_source empty(std::nostopstate);
  assert(!empty.stop_possible());
  assert(!empty.request_stop());
}
