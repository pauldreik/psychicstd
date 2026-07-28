#include "psyassert.h"
#include <iterator>
#include <vector>

struct bidirectional_iterator {
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = int;
  using difference_type = std::ptrdiff_t;
  using pointer = int*;
  using reference = int&;

  int* value;
  bidirectional_iterator& operator++() {
    ++value;
    return *this;
  }
  bidirectional_iterator& operator--() {
    --value;
    return *this;
  }
};

struct derived_reverse_iterator : std::reverse_iterator<int*> {
  using std::reverse_iterator<int*>::reverse_iterator;
  int* underlying() const { return current; }
};

int main() {
  std::initializer_list<int> init_values{1, 2, 3};
  psyassert(std::data(init_values) == init_values.begin());

#if !defined(PSYCHICSTD_TEST_PSYCHICSTD) ||                                    \
    _PSYCHICSTD_COMPATIBILITY_LEVEL >= _PSYCHICSTD_COMPAT_DROPIN
  static_assert(std::input_iterator<int*>);
#endif

  std::vector<int> v = {1, 2, 3};
  psyassert(*std::begin(v) == 1);
  psyassert(std::ssize(v) == 3);
  int values[2]{};
  psyassert(std::ssize(values) == 2);

  int bidirectional_values[] = {1, 2, 3};
  bidirectional_iterator it{bidirectional_values + 2};
  std::advance(it, -2);
  psyassert(it.value == bidirectional_values);

  derived_reverse_iterator reverse(values + 2);
  psyassert(reverse.underlying() == values + 2);
}
