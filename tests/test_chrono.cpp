#include "psyassert.h"
#include <chrono>
#include <cmath>
#include <limits>

using namespace std::chrono;

static void test_duration_cast_same_period() {
  // Converting nanoseconds to nanoseconds with a large value should not
  // overflow.  The intermediate d.count() * Period::num * ToPer::den
  // can overflow int64 even when the periods are identical (num == den).
  // Catch2's catch_timer.cpp hits this: getCurrentNanosecondsSinceEpoch()
  // calls duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).
  nanoseconds large(63'514'168'108'767LL); // ~17.6 hours in ns
  auto result = duration_cast<nanoseconds>(large);
  psyassert(result.count() == 63'514'168'108'767LL);
}

static void test_duration_cast_identity() {
  milliseconds ms(42);
  auto ns = duration_cast<nanoseconds>(ms);
  psyassert(ns.count() == 42'000'000);
}

static void test_duration_cast_down() {
  nanoseconds ns(1'500'000'000);
  auto sec = duration_cast<seconds>(ns);
  psyassert(sec.count() == 1);
}

static void test_duration_cast_up() {
  seconds sec(2);
  auto ms = duration_cast<milliseconds>(sec);
  psyassert(ms.count() == 2'000);
}

static void test_duration_cast_wide_intermediate() {
  using attoseconds = duration<__int128, std::atto>;
  const auto lo = nanoseconds::min();
  const auto hi = nanoseconds::max();
  psyassert(
      duration_cast<nanoseconds>(duration_cast<attoseconds>(lo)).count() ==
      lo.count());
  psyassert(
      duration_cast<nanoseconds>(duration_cast<attoseconds>(hi)).count() ==
      hi.count());
}

static void test_duration_fractional_conversion() {
  milliseconds ms(1230);
  duration<double> seconds(ms);
  psyassert(seconds.count() == 1.23);
}

static void test_duration_scalar_remainder_promotes_rep() {
  using int_seconds = duration<int>;
  using result_type = decltype(int_seconds{5} % 2LL);
  static_assert(std::is_same_v<result_type, duration<long long>>);
  static_assert((int_seconds{5} % 2LL).count() == 1);
}

static void test_time_point_arithmetic() {
  time_point<system_clock, milliseconds> epoch(milliseconds(0));
  psyassert((epoch + milliseconds(5)).time_since_epoch().count() == 5);
  psyassert((epoch - milliseconds(2)).time_since_epoch().count() == -2);
  time_point<system_clock, seconds> later(seconds(2));
  psyassert(epoch < later);
  psyassert(later > epoch);
  psyassert(epoch <= later);
  psyassert(later >= epoch);
  psyassert(epoch != later);
  psyassert((epoch == time_point<system_clock, microseconds>(microseconds(0))));
}

static void test_duration_bounds() {
  static_assert(seconds::min().count() ==
                std::numeric_limits<long long>::min());
  static_assert(seconds::max().count() ==
                std::numeric_limits<long long>::max());
  using unsigned_seconds = duration<unsigned>;
  static_assert(unsigned_seconds::min().count() == 0);
  static_assert(unsigned_seconds::max().count() ==
                std::numeric_limits<unsigned>::max());
  using float_seconds = duration<float>;
  static_assert(float_seconds::min().count() ==
                std::numeric_limits<float>::lowest());
  static_assert(float_seconds::max().count() ==
                std::numeric_limits<float>::max());
}

static void test_duration_increment_decrement() {
  static_assert([] {
    seconds s(5);
    const seconds pre_inc = ++s;
    bool ok = pre_inc.count() == 6 && s.count() == 6;
    const seconds post_inc = s++;
    ok = ok && post_inc.count() == 6 && s.count() == 7;
    const seconds pre_dec = --s;
    ok = ok && pre_dec.count() == 6 && s.count() == 6;
    const seconds post_dec = s--;
    ok = ok && post_dec.count() == 6 && s.count() == 5;
    return ok;
  }());
}
static void test_integral_is_finite() {
  static_assert(std::isfinite(0));
  static_assert(!std::isinf(0LL));
  static_assert(!std::isnan(0U));
}

int main() {
  test_duration_scalar_remainder_promotes_rep();
  const std::chrono::sys_seconds epoch{std::chrono::seconds{0}};
  const auto epoch_days = std::chrono::floor<std::chrono::days>(epoch);
  const std::chrono::year_month_day epoch_date{epoch_days};
  psyassert(static_cast<int>(epoch_date.year()) == 1970);
  psyassert(static_cast<unsigned>(epoch_date.month()) == 1);
  psyassert(static_cast<unsigned>(epoch_date.day()) == 1);
  const std::chrono::year_month_day leap{
      std::chrono::year{2000}, std::chrono::month{2}, std::chrono::day{29}};
  psyassert(leap.ok());
  psyassert(std::chrono::sys_days{leap}.time_since_epoch().count() == 11016);
  const std::chrono::hh_mm_ss time{std::chrono::seconds{3723}};
  psyassert(time.hours().count() == 1);
  psyassert(time.minutes().count() == 2);
  psyassert(time.seconds().count() == 3);
  const std::chrono::hh_mm_ss negative_time{std::chrono::seconds{-3723}};
  psyassert(negative_time.hours().count() == 1);
  psyassert(negative_time.minutes().count() == 2);
  psyassert(negative_time.seconds().count() == 3);
  using namespace std::chrono_literals;
  static_assert(2h == std::chrono::hours(2));
  static_assert(3min == std::chrono::minutes(3));
  static_assert(4s == std::chrono::seconds(4));
  static_assert(5ms == std::chrono::milliseconds(5));
  static_assert(6us == std::chrono::microseconds(6));
  static_assert(7ns == std::chrono::nanoseconds(7));
  static_assert(10ms / 2ms == 5);
  static_assert(2s / 500ms == 4);
  static_assert(1500ms % 1s == 500ms);
  static_assert(1500ms % 1000 == 500ms);
  constexpr auto remainder = [] {
    auto value = 1500ms;
    value %= 1s;
    value %= 300;
    return value;
  }();
  static_assert(remainder == 200ms);

  test_duration_cast_identity();
  test_duration_cast_down();
  test_duration_cast_up();
  test_duration_cast_wide_intermediate();
  test_duration_fractional_conversion();
  test_duration_cast_same_period();
  test_time_point_arithmetic();
  test_duration_bounds();
  test_duration_increment_decrement();
  test_integral_is_finite();
}
