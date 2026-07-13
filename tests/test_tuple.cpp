#include "psyassert.h"
#include <string_view>
#include <tuple>

namespace {
struct S {
  bool operator==(const S& other) const { return v == other.v; }
  int v = 42;
};

struct Tracker {
  int copies = 0, moves = 0;
  Tracker() = default;
  Tracker(const Tracker& o) : copies(o.copies + 1), moves(o.moves) {}
  Tracker(Tracker&& o) noexcept : copies(o.copies), moves(o.moves + 1) {}
};
struct Wrapper {
  Tracker t;
  Wrapper() = default;
  Wrapper(const Tracker& x) : t(x) {}
  Wrapper(Tracker&& x) : t(static_cast<Tracker&&>(x)) {}
};

std::tuple<const char*, int, bool> make_result(const char* p, int n) {
  return {p, n, false};
}
} // namespace

int main() {
  auto r = make_result("hi", 3);
  psyassert(std::get<0>(r) == std::string_view("hi"));
  psyassert(std::get<1>(r) == 3);
  psyassert(std::get<2>(r) == false);

  auto t = std::make_tuple(1, 'a', 3.14);
  psyassert(std::get<0>(t) == 1);
  std::get<0>(t) = 2;
  psyassert(std::get<0>(t) == 2);

  for (int i = 0; i < 2; ++i) {
    std::tuple<int, S> t2(i, S{i + 100});

    psyassert(std::get<0>(t2) == i);
    psyassert(std::get<1>(t2) == S{i + 100});
    psyassert(std::get<int>(t2) == i);
    psyassert(std::get<S>(t2) == S{i + 100});

    const auto& t2ref = t2;
    psyassert(std::get<0>(t2ref) == i);
    psyassert(std::get<1>(t2ref) == S{i + 100});
    psyassert(std::get<int>(t2ref) == i);
    psyassert(std::get<S>(t2ref) == S{i + 100});

    psyassert(std::get<S>(decltype(t2){}).v == 42);
  }

  {
    std::tuple<int> single{1};
    std::tuple<int> copy(single); // direct-init copy from non-const lvalue
    psyassert(std::get<0>(copy) == 1);
  }

  {
    // Heterogeneous move-converting construction must move, not copy.
    std::tuple<Tracker> src{};
    std::tuple<Wrapper> dst(std::move(src));
    psyassert(std::get<0>(dst).t.moves == 1);
    psyassert(std::get<0>(dst).t.copies == 0);
  }
}
