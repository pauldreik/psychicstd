#include "psyassert.h"
#include <iterator>
#include <vector>

int main() {
  std::vector<int> v = {1, 2, 3};
  psyassert(*std::begin(v) == 1);
}
