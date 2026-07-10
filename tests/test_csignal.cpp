#include <cassert>
#include <csignal>

namespace {
volatile std::sig_atomic_t got_signal = 0;
void handler(int) { got_signal = 1; }
} // namespace

int main() {
  auto prev = std::signal(SIGUSR1, handler);
  assert(prev != SIG_ERR);

  assert(got_signal == 0);
  std::raise(SIGUSR1);
  assert(got_signal == 1);

  std::signal(SIGUSR1, SIG_DFL);
}
