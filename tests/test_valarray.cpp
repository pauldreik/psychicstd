#include "psyassert.h"
#include <valarray>

int main() {
  std::valarray<int> v = {1, 2, 3};
  psyassert(v.size() == 3);
}
