#include "psyassert.h"
#include <future>
#include <vector>

int main() {
  std::vector<std::future<unsigned int>> futures;
  for (unsigned int i = 0; i < 4; ++i)
    futures.emplace_back(std::async(
        std::launch::async, [](unsigned int value) { return value + 1; }, i));
  unsigned int total = 0;
  for (auto& future : futures)
    total += future.get();
  psyassert(total == 10);
}
