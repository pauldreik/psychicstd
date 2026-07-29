#include "psyassert.h"
#include <compare>
#include <type_traits>

struct no_comparison {};

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

int main() {
  psyassert(std::strong_ordering::equal == 0);
  psyassert((std::strong_ordering::less <=> 0) == std::strong_ordering::less);
  psyassert((0 <=> std::weak_ordering::less) == std::weak_ordering::greater);
  psyassert((0 <=> std::partial_ordering::unordered) ==
            std::partial_ordering::unordered);
}
