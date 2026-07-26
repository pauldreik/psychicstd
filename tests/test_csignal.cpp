#include "psyassert.h"
#include <csignal>

int main() {
  auto* signal_function = &std::signal;
  auto* raise_function = &std::raise;
  psyassert(signal_function != nullptr);
  psyassert(raise_function != nullptr);
}
