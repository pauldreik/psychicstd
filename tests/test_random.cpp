#include "psyassert.h"
#include <random>

int main() {
  std::random_device rd;
  auto v = rd();
  psyassert(v >= rd.min());

  std::mt19937 gen(1234);
  std::mt19937 same_gen(1234);
  psyassert(gen == same_gen);
  same_gen();
  psyassert(!(gen == same_gen));
  long seed = 5678;
  gen.seed(seed);

  std::bernoulli_distribution never(0);
  std::bernoulli_distribution always(1);
  for (int i = 0; i < 10; ++i) {
    psyassert(!never(gen));
    psyassert(always(gen));
  }

  std::discrete_distribution<> d({1.0, 0.0, 0.0});
  for (int i = 0; i < 10; ++i)
    psyassert(d(gen) == 0);

  std::uniform_int_distribution<uint16_t> full_range(0, 0xFFFF);
  for (int i = 0; i < 1000; ++i)
    full_range(gen);
}
