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
  int identity_value = 42;
  static_assert(
      std::is_same_v<decltype(std::identity{}(identity_value)), int&>);
  psyassert(&std::identity{}(identity_value) == &identity_value);

  auto h = std::hash<const char*>{};
  psyassert(h("hello") != 0);

  std::function<int()> first = [] { return 1; };
  std::function<int()> second = [] { return 2; };
  first.swap(second);
  psyassert(first() == 2);
  psyassert(second() == 1);
  int referred = 3;
  std::function<int&(int*)> reference = [](int* value) -> int& {
    return *value;
  };
  psyassert(&reference(&referred) == &referred);

  member_target target;
  psyassert(std::invoke(&member_target::value, target) == 4);
  psyassert(std::invoke(&member_target::value, &target) == 4);
  psyassert(std::invoke(&member_target::add, target, 3) == 7);
  std::function<int(member_target, int)> member_function = &member_target::add;
  psyassert(member_function(target, 3) == 7);
  static_assert(
      std::is_same_v<
          std::invoke_result_t<decltype(&member_target::value), member_target&>,
          int&>);

  const auto positive = std::bind(std::less<int>{}, 0, 2);
  psyassert(positive());
  const auto subtract = std::bind(std::minus<int>{}, std::placeholders::_2,
                                  std::placeholders::_1);
  psyassert(subtract(3, 8) == 5);
  psyassert(std::bit_and<unsigned>{}(6, 3) == 2);
  psyassert(std::bit_or<unsigned>{}(4, 1) == 5);
  psyassert(std::bit_xor<unsigned>{}(6, 3) == 5);
  psyassert(std::bit_not<unsigned>{}(0) == ~0U);

  (void)std::ref(no_args)();
}
