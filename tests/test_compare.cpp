#include "psyassert.h"
#include <compare>

struct strongly_ordered {
  int value;
  constexpr auto operator<=>(const strongly_ordered&) const = default;
};

struct not_ordered {};

static_assert(
    std::three_way_comparable<strongly_ordered, std::strong_ordering>);
static_assert(!std::three_way_comparable<not_ordered, std::strong_ordering>);

int main() {
  psyassert(std::strong_ordering::equal == 0);
  psyassert((std::strong_ordering::less <=> 0) == std::strong_ordering::less);
  psyassert((0 <=> std::weak_ordering::less) == std::weak_ordering::greater);
  psyassert((0 <=> std::partial_ordering::unordered) ==
            std::partial_ordering::unordered);
}
