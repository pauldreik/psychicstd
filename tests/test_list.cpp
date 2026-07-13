#include "psyassert.h"
#include <list>

int main() {
  std::list<int> l = {1, 2, 3};
  psyassert(l.front() == 1);
}
