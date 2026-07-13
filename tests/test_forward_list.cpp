#include "psyassert.h"
#include <forward_list>

int main() {
  std::forward_list<int> fl = {1, 2, 3};
  psyassert(fl.front() == 1);
}
