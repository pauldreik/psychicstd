#include <cassert>
#include <stack>

int main() {
  std::stack<int> s;
  s.push(42);
  assert(s.top() == 42);

  std::stack<int> other;
  other.push(7);
  swap(s, other);
  assert(s.top() == 7);
  assert(other.top() == 42);
}
