#include "psyassert.h"
#include <atomic>
#include <stop_token>
#include <utility>

int main() {
  std::stop_source source;
  std::stop_token token = source.get_token();
  psyassert(source.stop_possible());
  psyassert(token.stop_possible());
  psyassert(!source.stop_requested());
  psyassert(!token.stop_requested());

  std::stop_token copied = token;
  std::stop_token moved = std::move(copied);
  psyassert(moved.stop_possible());
  psyassert(copied.stop_possible() == false);

  std::atomic<int> callback_runs(0);
  {
    std::stop_callback callback(token, [&] { ++callback_runs; });
    bool requested = source.request_stop();
    psyassert(requested);
    psyassert(callback_runs == 1);
  }
  psyassert(source.stop_requested());
  psyassert(token.stop_requested());
  psyassert(moved.stop_requested());

  std::atomic<int> late_runs(0);
  std::stop_callback late(token, [&] { ++late_runs; });
  psyassert(late_runs == 1);

  std::stop_source empty(std::nostopstate);
  psyassert(!empty.stop_possible());
  psyassert(!empty.request_stop());
}
