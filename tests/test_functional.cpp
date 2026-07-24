#include "psyassert.h"
#include <cstring>
#include <functional>

struct incomplete;
template <typename T> struct holder {
  T value;
};
holder<incomplete>* no_args() { return nullptr; }

struct member_target {
  int value = 4;
  int add(int amount) const { return value + amount; }
};

int main() {
  auto h = std::hash<const char*>{};
  psyassert(h("hello") != 0);

  std::function<int()> first = [] { return 1; };
  std::function<int()> second = [] { return 2; };
  first.swap(second);
  psyassert(first() == 2);
  psyassert(second() == 1);

  member_target target;
  psyassert(std::invoke(&member_target::value, target) == 4);
  psyassert(std::invoke(&member_target::value, &target) == 4);
  psyassert(std::invoke(&member_target::add, target, 3) == 7);
  static_assert(
      std::is_same_v<
          std::invoke_result_t<decltype(&member_target::value), member_target&>,
          int&>);

  (void)std::ref(no_args)();
}
