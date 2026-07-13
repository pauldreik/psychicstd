#include "psyassert.h"
#include <optional>

struct move_only {
  int value;

  explicit move_only(int v) : value(v) {}
  move_only(const move_only&) = delete;
  move_only& operator=(const move_only&) = delete;
  move_only(move_only&& o) noexcept : value(o.value) { o.value = 0; }
  move_only& operator=(move_only&& o) noexcept {
    value = o.value;
    o.value = 0;
    return *this;
  }
};

int main() {
  std::optional<int> o = 42;
  psyassert(o.value() == 42);

  std::optional<move_only> a(move_only(7));
  std::optional<move_only> b(static_cast<decltype(a)&&>(a));
  psyassert(b->value == 7);

  a.emplace(8);
  b = static_cast<decltype(a)&&>(a);
  psyassert(b->value == 8);

  auto c = std::make_optional(move_only(9));
  psyassert(c->value == 9);
}
