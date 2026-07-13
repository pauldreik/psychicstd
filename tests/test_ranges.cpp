#include "psyassert.h"
#include <ranges>
#include <vector>

namespace custom {
inline int swaps;

struct iterator {
  int* p;
  int& operator*() const { return *p; }
};

int iter_move(iterator i) { return *i + 10; }
void iter_swap(iterator a, iterator b) {
  ++swaps;
  int tmp = *a;
  *a = *b;
  *b = tmp;
}
} // namespace custom

int main() {
  std::vector<int> v = {1, 2, 3};
  psyassert(*std::ranges::begin(v) == 1);

  int a = 4;
  int b = 5;
  std::ranges::iter_swap(&a, &b);
  psyassert(a == 5 && b == 4);
  psyassert(std::ranges::iter_move(&a) == 5);

  custom::iterator ai{&a};
  custom::iterator bi{&b};
  psyassert(std::ranges::iter_move(ai) == 15);
  std::ranges::iter_swap(ai, bi);
  psyassert(custom::swaps == 1);
  psyassert(a == 4 && b == 5);
}
