#include "psyassert.h"
#include <type_traits>

using Fn = void();
using NothrowFn = int(int) noexcept;
using ConstFn = void() const;
struct DeletedDestructor {
  ~DeletedDestructor() = delete;
};

struct Pod {
  int value;
};

struct NonPod {
  NonPod() : value(0) {}
  int value;
};

struct ReturnsReference {
  int& operator()(int (&&value)[2]) const { return value[0]; }
};
struct NothrowCallable {
  int operator()(int) const noexcept { return 0; }
  int value = 0;
};
struct MutableCallable {
  void operator()() {}
};

int main() {
  static_assert(std::is_same_v<int, int>);
  static_assert(std::is_same_v<const int, const int>);
  static_assert(!std::is_same_v<int, const int>);
  static_assert(!std::is_same_v<int, long>);
  static_assert(std::is_pod_v<Pod>);
  static_assert(!std::is_pod_v<NonPod>);
  static_assert(std::is_function_v<Fn>);
  static_assert(
      std::is_same_v<std::decay_t<NothrowFn&>, int (*)(int) noexcept>);
  static_assert(std::is_function_v<ConstFn>);
  static_assert(!std::is_function_v<int>);
  static_assert(!std::is_destructible_v<DeletedDestructor>);
  static_assert(!std::is_trivially_destructible_v<DeletedDestructor>);
  static_assert(std::is_trivially_constructible_v<int>);
  static_assert(std::is_trivially_constructible_v<int, int>);
  static_assert(!std::is_trivially_constructible_v<int, int, int>);
  static_assert(std::is_trivially_assignable_v<int&, int>);
  static_assert(!std::is_trivially_assignable_v<const int&, int>);
  static_assert(std::is_unsigned_v<std::make_unsigned_t<wchar_t>>);
  static_assert(sizeof(std::make_unsigned_t<wchar_t>) == sizeof(wchar_t));
  static_assert(std::is_signed_v<std::make_signed_t<char16_t>>);
  static_assert(sizeof(std::make_signed_t<char16_t>) == sizeof(char16_t));
  static_assert(std::is_invocable_v<ReturnsReference, int[2]>);
  static_assert(std::is_invocable_r_v<int&, ReturnsReference, int[2]>);
  static_assert(std::is_nothrow_invocable_v<NothrowCallable, int>);
  static_assert(std::is_nothrow_invocable_r_v<int, NothrowCallable, int>);
  static_assert(std::is_nothrow_invocable_v<decltype(&NothrowCallable::value),
                                            const NothrowCallable&&>);
  static_assert(!std::is_nothrow_invocable_v<int, int>);
  static_assert(std::is_invocable_v<MutableCallable&>);
  static_assert(!std::is_invocable_v<const MutableCallable&>);
  static_assert(std::alignment_of<int>::value == alignof(int));
  static_assert(std::alignment_of_v<int> == alignof(int));
  static_assert(std::negation_v<std::false_type>);
  static_assert(!std::negation_v<std::true_type>);
}
