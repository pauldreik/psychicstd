#include "psyassert.h"
#include <cstring>
#include <functional>
#include <memory>

struct incomplete;
template <typename T> struct holder {
  T value;
};
holder<incomplete>* no_args() { return nullptr; }

struct member_target {
  int value = 4;
  int add(int amount) const { return value + amount; }
};

struct conflicting_wrapper {
  member_target* dereferenced;
  member_target* returned;
  member_target& operator*() const { return *dereferenced; }
  member_target& get() const { return *returned; }
};

enum class hashable_enum : unsigned { value = 42 };

struct immovable_result {
  int value;
  explicit immovable_result(int value_arg) : value(value_arg) {}
  immovable_result(const immovable_result&) = delete;
  immovable_result(immovable_result&&) = delete;
};

int main() {
  psyassert(std::divides<>{}(8.0, 2.0) == 4.0);
  psyassert(std::negate<>{}(3) == -3);
  static_assert(requires { typename std::negate<>::is_transparent; });

  int identity_value = 42;
  static_assert(
      std::is_same_v<decltype(std::identity{}(identity_value)), int&>);
  psyassert(&std::identity{}(identity_value) == &identity_value);

  auto h = std::hash<const char*>{};
  psyassert(h("hello") != 0);
  psyassert(std::hash<hashable_enum>{}(hashable_enum::value) ==
            std::hash<unsigned>{}(42));

  std::function<int()> first = [] { return 1; };
  std::function<int()> second = [] { return 2; };
  first.swap(second);
  psyassert(first() == 2);
  psyassert(second() == 1);
  int discarded_result_calls = 0;
  std::function<void()> discard_result = [&discarded_result_calls] {
    ++discarded_result_calls;
    return 42;
  };
  discard_result();
  psyassert(discarded_result_calls == 1);
  std::function<immovable_result()> return_immovable = [] {
    return immovable_result{42};
  };
  psyassert(return_immovable().value == 42);
  struct callable {
    int value;
    int operator()() const { return value; }
  };
  std::function<int()> targeted = callable{3};
  psyassert(targeted.target<callable>() != nullptr);
  targeted.target<callable>()->value = 4;
  psyassert(targeted() == 4);
  psyassert(targeted.target<int (*)()>() == nullptr);
  const auto& const_targeted = targeted;
  psyassert(const_targeted.target<callable>()->value == 4);
  int (*null_function)() = nullptr;
  std::function<int()> empty_function = null_function;
  psyassert(!empty_function);
  psyassert(empty_function.target<int (*)()>() == nullptr);
  std::function<long()> converted_empty = empty_function;
  psyassert(!converted_empty);
  int referred = 3;
  std::function<int&(int*)> reference = [](int* value) -> int& {
    return *value;
  };
  psyassert(&reference(&referred) == &referred);

  member_target target;
  psyassert(std::invoke(&member_target::value, target) == 4);
  psyassert(std::invoke(&member_target::value, &target) == 4);
  psyassert(std::invoke(&member_target::add, target, 3) == 7);
  auto shared_target = std::make_shared<member_target>();
  psyassert(std::invoke(&member_target::value, shared_target) == 4);
  psyassert(std::invoke(&member_target::add, shared_target, 3) == 7);
  psyassert(std::invoke(&member_target::add, std::ref(target), 3) == 7);
  member_target other_target{9};
  conflicting_wrapper wrapper{&target, &other_target};
  psyassert(std::invoke(&member_target::value, wrapper) == 4);
  psyassert(std::invoke(&member_target::add, wrapper, 3) == 7);
  std::function<int(member_target, int)> member_function = &member_target::add;
  psyassert(member_function(target, 3) == 7);
  int (member_target::*null_member_function)(int) const = nullptr;
  std::function<int(member_target, int)> empty_member_function =
      null_member_function;
  psyassert(!empty_member_function);
  int member_target::* null_member_object = nullptr;
  std::function<int&(member_target&)> empty_member_object = null_member_object;
  psyassert(!empty_member_object);
  static_assert(
      std::is_same_v<
          std::invoke_result_t<decltype(&member_target::value), member_target&>,
          int&>);
  static_assert(std::is_invocable_v<decltype(&member_target::add),
                                    std::shared_ptr<member_target>&, int>);
  static_assert(
      std::is_same_v<std::invoke_result_t<decltype(&member_target::value),
                                          std::shared_ptr<member_target>&>,
                     int&>);
  psyassert(std::mem_fn(&member_target::value)(target) == 4);
  psyassert(std::mem_fn(&member_target::add)(&target, 3) == 7);

  const auto positive = std::bind(std::less<int>{}, 0, 2);
  psyassert(positive());
  const auto subtract = std::bind(std::minus<int>{}, std::placeholders::_2,
                                  std::placeholders::_1);
  psyassert(subtract(3, 8) == 5);
  const auto bound_member =
      std::bind(&member_target::add, shared_target, std::placeholders::_1);
  static_assert(std::is_invocable_r_v<int, decltype(bound_member), int>);
  std::function<int(int)> bound_member_function = bound_member;
  psyassert(bound_member_function(3) == 7);
  const auto constrained =
      std::bind([](int value) { return value; }, std::placeholders::_1);
  static_assert(std::is_invocable_v<decltype(constrained), int>);
  static_assert(!std::is_invocable_v<decltype(constrained), const char*>);
  const auto nested = std::bind(
      [](int value) { return value; },
      std::bind([](int value) { return value; }, std::placeholders::_1));
  static_assert(std::is_invocable_v<decltype(nested), int>);
  static_assert(!std::is_invocable_v<decltype(nested), const char*>);
  int bind_void_calls = 0;
  std::bind<void>([&bind_void_calls](int) { ++bind_void_calls; }, 1)();
  psyassert(bind_void_calls == 1);
  psyassert(!std::logical_not<>{}(true));
  psyassert(std::modulus<>{}(7, 4) == 3);
  psyassert(std::bit_and<unsigned>{}(6, 3) == 2);
  psyassert(std::bit_or<unsigned>{}(4, 1) == 5);
  psyassert(std::bit_xor<unsigned>{}(6, 3) == 5);
  psyassert(std::bit_not<unsigned>{}(0) == ~0U);

  (void)std::ref(no_args)();
}
