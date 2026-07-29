#include "psyassert.h"
#include <atomic>
#include <memory>
#include <optional>
#include <stop_token>
#include <utility>

struct callback_holder;

struct delete_callback {
  callback_holder& holder;
  void operator()() const;
};

struct callback_holder {
  std::unique_ptr<std::stop_callback<delete_callback>> callback;
};

void delete_callback::operator()() const { holder.callback.reset(); }

struct count_callback {
  int& count;
  void operator()() const { ++count; }
};

int main() {
  std::optional<std::stop_source> discarded_source(std::in_place);
  std::stop_token source_less_token = discarded_source->get_token();
  discarded_source.reset();
  psyassert(!source_less_token.stop_possible());

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

  std::stop_source self_delete_source;
  callback_holder holder;
  holder.callback = std::make_unique<std::stop_callback<delete_callback>>(
      self_delete_source.get_token(), delete_callback{holder});
  psyassert(self_delete_source.request_stop());
  psyassert(holder.callback == nullptr);

  std::stop_source remove_pending_source;
  int removed_runs = 0;
  std::optional<std::stop_callback<count_callback>> removed(
      std::in_place, remove_pending_source.get_token(),
      count_callback{removed_runs});
  std::stop_callback remove_pending(remove_pending_source.get_token(),
                                    [&] { removed.reset(); });
  psyassert(remove_pending_source.request_stop());
  psyassert(removed_runs == 0);
}
