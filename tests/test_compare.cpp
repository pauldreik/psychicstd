#include "psyassert.h"
#include <compare>
#include <type_traits>

struct no_comparison {};

template <typename Ordering> constexpr bool has_ordering_surface() {
  static_assert(std::is_same_v<decltype(Ordering::less == 0), bool>);
  static_assert(std::is_same_v<decltype(0 == Ordering::less), bool>);
  static_assert(std::is_same_v<decltype(Ordering::less != 0), bool>);
  static_assert(std::is_same_v<decltype(0 != Ordering::less), bool>);
  static_assert(std::is_same_v<decltype(Ordering::less < 0), bool>);
  static_assert(std::is_same_v<decltype(0 < Ordering::less), bool>);
  static_assert(std::is_same_v<decltype(Ordering::less > 0), bool>);
  static_assert(std::is_same_v<decltype(0 > Ordering::less), bool>);
  static_assert(std::is_same_v<decltype(Ordering::less <= 0), bool>);
  static_assert(std::is_same_v<decltype(0 <= Ordering::less), bool>);
  static_assert(std::is_same_v<decltype(Ordering::less >= 0), bool>);
  static_assert(std::is_same_v<decltype(0 >= Ordering::less), bool>);
  static_assert(std::is_same_v<decltype(Ordering::less <=> 0), Ordering>);
  static_assert(std::is_same_v<decltype(0 <=> Ordering::less), Ordering>);

  static_assert(noexcept(Ordering::less == 0));
  static_assert(noexcept(0 == Ordering::less));
  static_assert(noexcept(Ordering::less != 0));
  static_assert(noexcept(0 != Ordering::less));
  static_assert(noexcept(Ordering::less < 0));
  static_assert(noexcept(0 < Ordering::less));
  static_assert(noexcept(Ordering::less > 0));
  static_assert(noexcept(0 > Ordering::less));
  static_assert(noexcept(Ordering::less <= 0));
  static_assert(noexcept(0 <= Ordering::less));
  static_assert(noexcept(Ordering::less >= 0));
  static_assert(noexcept(0 >= Ordering::less));
  static_assert(noexcept(Ordering::less <=> 0));
  static_assert(noexcept(0 <=> Ordering::less));
  return true;
}

template <typename Ordering>
constexpr bool check_ordering(Ordering value, bool equal, bool less,
                              bool greater, bool less_equal,
                              bool greater_equal) {
  return (value == 0) == equal && (0 == value) == equal &&
         (value != 0) == !equal && (0 != value) == !equal &&
         (value < 0) == less && (0 < value) == greater &&
         (value > 0) == greater && (0 > value) == less &&
         (value <= 0) == less_equal && (0 <= value) == greater_equal &&
         (value >= 0) == greater_equal && (0 >= value) == less_equal;
}

template <typename T>
concept has_compare_three_way_result =
    requires { typename std::compare_three_way_result<T>::type; };

template <template <typename, typename> typename> struct takes_binary_template;
using compare_result_is_binary =
    takes_binary_template<std::compare_three_way_result>;

static_assert(
    std::is_same_v<std::compare_three_way_result_t<int>, std::strong_ordering>);
static_assert(std::is_same_v<std::compare_three_way_result_t<double, int>,
                             std::partial_ordering>);
static_assert(!has_compare_three_way_result<no_comparison>);

static_assert(has_ordering_surface<std::partial_ordering>());
static_assert(has_ordering_surface<std::weak_ordering>());
static_assert(has_ordering_surface<std::strong_ordering>());

static_assert(check_ordering(std::partial_ordering::less, false, true, false,
                             true, false));
static_assert(check_ordering(std::partial_ordering::equivalent, true, false,
                             false, true, true));
static_assert(check_ordering(std::partial_ordering::greater, false, false, true,
                             false, true));
static_assert(check_ordering(std::partial_ordering::unordered, false, false,
                             false, false, false));
static_assert(check_ordering(std::weak_ordering::less, false, true, false, true,
                             false));
static_assert(check_ordering(std::weak_ordering::equivalent, true, false, false,
                             true, true));
static_assert(check_ordering(std::weak_ordering::greater, false, false, true,
                             false, true));
static_assert(check_ordering(std::strong_ordering::less, false, true, false,
                             true, false));
static_assert(check_ordering(std::strong_ordering::equal, true, false, false,
                             true, true));
static_assert(check_ordering(std::strong_ordering::equivalent, true, false,
                             false, true, true));
static_assert(check_ordering(std::strong_ordering::greater, false, false, true,
                             false, true));

static_assert(static_cast<std::partial_ordering>(std::weak_ordering::less) ==
              std::partial_ordering::less);
static_assert(
    static_cast<std::partial_ordering>(std::strong_ordering::greater) ==
    std::partial_ordering::greater);
static_assert(static_cast<std::weak_ordering>(std::strong_ordering::equal) ==
              std::weak_ordering::equivalent);

static_assert(std::is_eq(std::strong_ordering::equal));
static_assert(std::is_neq(std::partial_ordering::unordered));
static_assert(std::is_lt(std::weak_ordering::less));
static_assert(std::is_gt(std::strong_ordering::greater));
static_assert(std::is_lteq(std::partial_ordering::equivalent));
static_assert(std::is_gteq(std::weak_ordering::equivalent));

static_assert(
    std::is_same_v<std::common_comparison_category_t<>, std::strong_ordering>);
static_assert(std::is_same_v<std::common_comparison_category_t<
                                 std::strong_ordering, std::weak_ordering>,
                             std::weak_ordering>);
static_assert(std::is_same_v<std::common_comparison_category_t<
                                 std::strong_ordering, std::partial_ordering>,
                             std::partial_ordering>);
static_assert(
    std::is_same_v<std::common_comparison_category_t<std::strong_ordering, int>,
                   void>);

static_assert(std::three_way_comparable<int>);
static_assert(std::three_way_comparable_with<int, long>);
static_assert(!std::three_way_comparable<no_comparison>);
static_assert(std::compare_three_way{}(1, 2) == std::strong_ordering::less);

int main() {
  psyassert(std::strong_ordering::equal == 0);
  psyassert((std::strong_ordering::less <=> 0) == std::strong_ordering::less);
  psyassert((0 <=> std::weak_ordering::less) == std::weak_ordering::greater);
  psyassert((0 <=> std::partial_ordering::unordered) ==
            std::partial_ordering::unordered);
}
