#include "psyassert.h"
#include <initializer_list>

int sum(std::initializer_list<int> il) {
  int s = 0;
  for (auto x : il)
    s += x;
  return s;
}

int main() { psyassert(sum({1, 2, 3}) == 6); }
