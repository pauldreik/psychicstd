#include "psyassert.h"
#include <string>
#include <variant>

using std::get;
using std::get_if;
using std::holds_alternative;
using std::monostate;
using std::variant;
using std::visit;

int main() {
  variant<int, std::string, double> v = 42;
  psyassert(v.index() == 0);
  psyassert(holds_alternative<int>(v));
  psyassert(get<int>(v) == 42);
  psyassert(get<0>(v) == 42);

  v = std::string("hello");
  psyassert(v.index() == 1);
  psyassert(get<std::string>(v) == "hello");
  psyassert(*get_if<std::string>(&v) == "hello");
  psyassert(get_if<int>(&v) == nullptr);

  v = 3.14;
  psyassert(v.index() == 2);
  psyassert(get<double>(v) == 3.14);

  variant<int, std::string> w(std::in_place_type<std::string>, 3, 'x');
  psyassert(get<std::string>(w) == "xxx");
  variant<int, std::string> w2(std::in_place_index<0>, 7);
  psyassert(get<0>(w2) == 7);

  w.emplace<int>(99);
  psyassert(get<int>(w) == 99);

  variant<int, std::string> a = std::string("a");
  variant<int, std::string> b = a;
  psyassert(get<std::string>(b) == "a");
  b = std::move(a);
  psyassert(get<std::string>(b) == "a");

  variant<int, double> num = 2;
  int r = visit([](auto x) { return (int)(x * 2); }, num);
  psyassert(r == 4);

  variant<int, std::string> c1 = 1;
  variant<int, std::string> c2 = 1;
  variant<int, std::string> c3 = 2;
  psyassert(c1 == c2);
  psyassert(c1 != c3);
  psyassert(c1 < c3);

  variant<monostate, int> m;
  psyassert(m.index() == 0);

  bool threw = false;
  try {
    (void)get<int>(v); // v holds double
  } catch (const std::bad_variant_access&) {
    threw = true;
  }
  psyassert(threw);

  static_assert(std::variant_size_v<variant<int, double, char>> == 3);
  static_assert(
      std::is_same_v<std::variant_alternative_t<1, variant<int, double>>,
                     double>);
  return 0;
}
