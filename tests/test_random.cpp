#include <cassert>
#include <random>

int main() {
  std::random_device rd;
  auto v = rd();
  assert(v >= rd.min());

  std::mt19937 gen(1234);
  long seed = 5678;
  gen.seed(seed);

  std::discrete_distribution<> d({1.0, 0.0, 0.0});
  for (int i = 0; i < 10; ++i)
    assert(d(gen) == 0);

  std::uniform_int_distribution<uint16_t> full_range(0, 0xFFFF);
  for (int i = 0; i < 1000; ++i)
    full_range(gen);
}
