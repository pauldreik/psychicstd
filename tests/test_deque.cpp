#include <cassert>
#include <deque>

int main() {
  std::deque<int> d;
  d.push_back(1);
  assert(d.front() == 1);
}
