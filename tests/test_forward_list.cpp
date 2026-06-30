#include <cassert>
#include <forward_list>

int main() {
  std::forward_list<int> fl = {1, 2, 3};
  assert(fl.front() == 1);
}
