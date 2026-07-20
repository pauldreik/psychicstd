#include "psyassert.h"
#include <type_traits>

using Fn = void();
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

int main() {
  static_assert(std::is_same_v<int, int>);
  static_assert(std::is_same_v<const int, const int>);
  static_assert(!std::is_same_v<int, const int>);
  static_assert(!std::is_same_v<int, long>);
  static_assert(std::is_pod_v<Pod>);
  static_assert(!std::is_pod_v<NonPod>);
  static_assert(std::is_function_v<Fn>);
  static_assert(std::is_function_v<ConstFn>);
  static_assert(!std::is_function_v<int>);
  static_assert(!std::is_destructible_v<DeletedDestructor>);
  static_assert(!std::is_trivially_destructible_v<DeletedDestructor>);
  static_assert(std::alignment_of<int>::value == alignof(int));
  static_assert(std::alignment_of_v<int> == alignof(int));
  static_assert(std::negation_v<std::false_type>);
  static_assert(!std::negation_v<std::true_type>);
}
