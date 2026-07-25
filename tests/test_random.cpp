#include "psyassert.h"
#include <limits>
#include <random>

int main() {
  std::minstd_rand0 zero_seed(0);
  std::minstd_rand0 one_seed(1);
  psyassert(zero_seed() == 16807);
  psyassert(one_seed() == 16807);
  zero_seed.seed();
  zero_seed.discard(9999);
  psyassert(zero_seed() == 1043618065);

  using full_width_engine = std::linear_congruential_engine<unsigned, 0, 0, 0>;
  full_width_engine full_width;
  full_width();

  unsigned seed_values[]{3, 5, 7};
  std::seed_seq lcg_seed(seed_values, seed_values + 3);
  std::linear_congruential_engine<unsigned, 5, 7, 11> seeded(lcg_seed);
  psyassert(seeded == decltype(seeded)(4));

  std::random_device rd;
  std::random_device token_rd("/dev/urandom");
  auto v = rd();
  psyassert(v >= rd.min());
  psyassert(token_rd() >= token_rd.min());

  std::mt19937 gen(1234);
  std::mt19937 same_gen(1234);
  psyassert(gen == same_gen);
  same_gen();
  psyassert(!(gen == same_gen));
  long seed = 5678;
  gen.seed(seed);

  std::seed_seq sequence{1, 2, 3};
  unsigned parameters[3]{};
  sequence.param(parameters);
  psyassert(parameters[0] == 1 && parameters[1] == 2 && parameters[2] == 3);
  std::mt19937 sequence_gen(sequence);
  std::mt19937_64 sequence_gen_64(sequence);
  std::minstd_rand sequence_minstd(sequence);
  sequence_gen.discard(2);
  sequence_gen_64.discard(2);
  sequence_minstd.discard(2);
  psyassert(sequence_gen() <= sequence_gen.max());
  psyassert(sequence_gen_64() <= sequence_gen_64.max());
  psyassert(sequence_minstd() <= sequence_minstd.max());

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

  std::uniform_int_distribution<> default_range;
  psyassert(default_range.min() == 0);
  psyassert(default_range.max() == std::numeric_limits<int>::max());
  bool generated_above_one = false;
  for (int i = 0; i < 10; ++i)
    generated_above_one |= default_range(gen) > 1;
  psyassert(generated_above_one);
}
