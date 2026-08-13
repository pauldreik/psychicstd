// Self-containment check: iter_swap/swap_ranges (the basis for sort,
// reverse, rotate, nth_element, push_heap/pop_heap/sort_heap, ...) must find
// the generic swap() from <algorithm> alone, without a container header
// (<vector>, <string>, ...) happening to pull it in transitively first.
#include "psyassert.h"
#include <algorithm>

int main() {
  int values[] = {5, 3, 1, 4, 2};
  std::sort(values, values + 5);
  psyassert(values[0] == 1 && values[4] == 5);

  std::reverse(values, values + 5);
  psyassert(values[0] == 5 && values[4] == 1);

  int unordered[] = {5, 3, 1, 4, 2};
  std::nth_element(unordered, unordered + 2, unordered + 5);
  psyassert(unordered[2] == 3);
}
