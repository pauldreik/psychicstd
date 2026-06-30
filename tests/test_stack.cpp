#include <cassert>
#include <stack>

int main() {
  std::stack<int> s;
  s.push(42);
  assert(s.top() == 42);
}
