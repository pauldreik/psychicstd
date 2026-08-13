#include "psyassert.h"
#include <type_traits>

using Fn = void();
using NothrowFn = int(int) noexcept;
using ConstFn = void() const;
using NothrowLvalueFn = int(int) & noexcept;
using NothrowConstRvalueFn = int(int) const&& noexcept;
using NothrowVariadicFn = int(int, ...) volatile& noexcept;
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
struct ImmovableResult {
  ImmovableResult() = default;
  ImmovableResult(const ImmovableResult&) = delete;
  ImmovableResult(ImmovableResult&&) = delete;
};
struct ReturnsImmovable {
  ImmovableResult operator()() const noexcept { return {}; }
};

int main() {
#ifdef __SIZEOF_INT128__
  static_assert(std::is_integral_v<__int128>);
  static_assert(std::is_integral_v<unsigned __int128>);
  static_assert(std::is_integral_v<const __int128>);
#endif
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
  static_assert(std::is_function_v<NothrowLvalueFn>);
  static_assert(std::is_function_v<NothrowConstRvalueFn>);
  static_assert(std::is_function_v<NothrowVariadicFn>);
  static_assert(!std::is_function_v<int>);
  static_assert(!std::is_function_v<int&>);
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
  static_assert(!std::is_convertible_v<ImmovableResult, ImmovableResult>);
  static_assert(std::is_invocable_r_v<ImmovableResult, ReturnsImmovable>);
  static_assert(
      std::is_nothrow_invocable_r_v<ImmovableResult, ReturnsImmovable>);
  static_assert(std::alignment_of<int>::value == alignof(int));
  static_assert(std::alignment_of_v<int> == alignof(int));
  static_assert(std::is_standard_layout_v<Pod>);
  static_assert(sizeof(std::aligned_storage_t<sizeof(int), alignof(int)>) ==
                sizeof(int));
  static_assert(alignof(std::aligned_storage_t<sizeof(int), alignof(int)>) ==
                alignof(int));
  static_assert(std::negation_v<std::false_type>);
  static_assert(!std::negation_v<std::true_type>);
  static_assert(std::rank_v<int> == 0);
  static_assert(std::rank_v<int[][2][3]> == 3);
  static_assert(std::extent_v<int[2][3]> == 2);
  static_assert(std::extent_v<int[2][3], 1> == 3);
}
