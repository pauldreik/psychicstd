#include "psyassert.h"
#include <cstring>
#include <functional>

struct incomplete;
template <typename T> struct holder {
  T value;
};
holder<incomplete>* no_args() { return nullptr; }

int main() {
  auto h = std::hash<const char*>{};
  psyassert(h("hello") != 0);

  std::function<int()> first = [] { return 1; };
  std::function<int()> second = [] { return 2; };
  first.swap(second);
  psyassert(first() == 2);
  psyassert(second() == 1);

  (void)std::ref(no_args)();
}
