#include "psyassert.h"
#include <stack>

int main() {
  std::stack<int> s;
  s.push(42);
  psyassert(s.top() == 42);

  std::stack<int> other;
  other.push(7);
  swap(s, other);
  psyassert(s.top() == 7);
  psyassert(other.top() == 42);
}
