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

int main() {
  std::vector<int> v = {1, 2, 3};
  psyassert(*std::begin(v) == 1);
  psyassert(std::ssize(v) == 3);
  int values[2]{};
  psyassert(std::ssize(values) == 2);

  int bidirectional_values[] = {1, 2, 3};
  bidirectional_iterator it{bidirectional_values + 2};
  std::advance(it, -2);
  psyassert(it.value == bidirectional_values);
}
