#include "psyassert.h"
#include <compare>

int main() {
  psyassert(std::strong_ordering::equal == 0);
  psyassert((std::strong_ordering::less <=> 0) == std::strong_ordering::less);
  psyassert((0 <=> std::weak_ordering::less) == std::weak_ordering::greater);
  psyassert((0 <=> std::partial_ordering::unordered) ==
            std::partial_ordering::unordered);
}
