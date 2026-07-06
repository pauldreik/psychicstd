#include <cassert>
#include <string_view>
#include <tuple>

namespace {
struct S {
  bool operator==(const S& other) const { return v == other.v; }
  int v = 42;
};

std::tuple<const char*, int, bool> make_result(const char* p, int n) {
  return {p, n, false};
}
} // namespace

int main() {
  auto r = make_result("hi", 3);
  assert(std::get<0>(r) == std::string_view("hi"));
  assert(std::get<1>(r) == 3);
  assert(std::get<2>(r) == false);

  auto t = std::make_tuple(1, 'a', 3.14);
  assert(std::get<0>(t) == 1);
  std::get<0>(t) = 2;
  assert(std::get<0>(t) == 2);

  for (int i = 0; i < 2; ++i) {
    std::tuple<int, S> t2(i, S{i + 100});

    assert(std::get<0>(t2) == i);
    assert(std::get<1>(t2) == S{i + 100});
    assert(std::get<int>(t2) == i);
    assert(std::get<S>(t2) == S{i + 100});

    const auto& t2ref = t2;
    assert(std::get<0>(t2ref) == i);
    assert(std::get<1>(t2ref) == S{i + 100});
    assert(std::get<int>(t2ref) == i);
    assert(std::get<S>(t2ref) == S{i + 100});

    assert(std::get<S>(decltype(t2){}).v == 42);
  }
}
