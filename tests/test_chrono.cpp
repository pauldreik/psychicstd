#include "psyassert.h"
#include <chrono>

#ifndef __cpp_lib_chrono
#error "<chrono> must define __cpp_lib_chrono"
#endif

#include <cmath>
#include <limits>
#include <type_traits>

using namespace std::chrono;

template <typename T>
concept has_chrono_abs = requires(T value) { std::chrono::abs(value); };

template <typename ToDuration, typename From>
concept has_chrono_round =
    requires(From value) { std::chrono::round<ToDuration>(value); };

static void test_duration_cast_paths() {
  // num == 1, den == 1: neither multiply nor divide. Cross-cancelling before
  // forming num and den also keeps these large period factors representable.
  using large_period = std::ratio<INTMAX_MAX / 2, INTMAX_MAX / 2 - 1>;
  using large_duration = std::chrono::duration<long long, large_period>;
  constexpr auto largest = std::numeric_limits<long long>::max();
  static_assert(
      duration_cast<large_duration>(large_duration(largest)).count() ==
      largest);

  // num == 1, den != 1: divide only. Multiplying this count by the
  // unreduced numerator of 1'000'000 overflows before the division.
  static_assert(duration_cast<microseconds>(nanoseconds(largest)).count() ==
                largest / 1'000);

  // num != 1, den == 1: multiply only. Converting floating-point seconds to
  // integral milliseconds requires duration_cast and truncates the result.
  static_assert(duration_cast<milliseconds>(duration<double>(1.2345)).count() ==
                1'234);

  // num != 1, den != 1: multiply and divide. The period ratio is 14/15.
  using two_thirds = duration<int, std::ratio<2, 3>>;
  using five_sevenths = duration<int, std::ratio<5, 7>>;
  static_assert(duration_cast<five_sevenths>(two_thirds(30)).count() == 28);
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

static void test_common_type() {
  using unusual = duration<long, std::ratio<1209600, 1000000>>;
  using common_duration = std::common_type_t<unusual, seconds>;
  using common_rep = std::common_type_t<unusual::rep, seconds::rep>;
  static_assert(std::is_same_v<common_duration,
                               duration<common_rep, std::ratio<1, 625>>>);

  using first = time_point<system_clock, unusual>;
  using second = time_point<system_clock, seconds>;
  static_assert(std::is_same_v<std::common_type_t<first, second>,
                               time_point<system_clock, common_duration>>);
}

static void test_time_point_arithmetic() {
  static_assert(
      !std::is_constructible_v<time_point<system_clock, seconds>,
                               time_point<system_clock, milliseconds>>);
  static_assert(std::is_constructible_v<time_point<system_clock, milliseconds>,
                                        time_point<system_clock, seconds>>);
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
  static_assert(sizeof(minutes::rep) >= 8);
  static_assert(sizeof(hours::rep) >= 8);
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

static void test_chrono_ceil_round_abs() {
  static_assert(floor<seconds>(milliseconds(1500)) == seconds(1));
  static_assert(floor<seconds>(milliseconds(2000)) == seconds(2));
  static_assert(floor<seconds>(milliseconds(-1500)) == seconds(-2));
  static_assert(ceil<seconds>(milliseconds(1500)) == seconds(2));
  static_assert(ceil<seconds>(milliseconds(2000)) == seconds(2));
  static_assert(ceil<seconds>(milliseconds(-1500)) == seconds(-1));
  static_assert(round<seconds>(milliseconds(1400)) == seconds(1));
  static_assert(round<seconds>(milliseconds(1600)) == seconds(2));
  static_assert(round<seconds>(milliseconds(-1400)) == seconds(-1));
  static_assert(round<seconds>(milliseconds(-1600)) == seconds(-2));
  // Exact tie rounds to even.
  static_assert(round<seconds>(milliseconds(1500)) == seconds(2));
  static_assert(round<seconds>(milliseconds(2500)) == seconds(2));
  static_assert(round<seconds>(milliseconds(-1500)) == seconds(-2));
  static_assert(round<seconds>(milliseconds(-2500)) == seconds(-2));
#ifdef PSYCHICSTD_TEST_PSYCHICSTD
  // Apple libc++ leaves these overloads visible to a requires-expression and
  // rejects them only when their bodies are instantiated.
  static_assert(!has_chrono_round<duration<double>, milliseconds>);
  using milliseconds_time_point = time_point<system_clock, milliseconds>;
  static_assert(!has_chrono_round<duration<double>, milliseconds_time_point>);
#endif
  static_assert(abs(seconds(-5)) == seconds(5));
  static_assert(abs(seconds(5)) == seconds(5));
  static_assert(!has_chrono_abs<duration<unsigned>>);

  const time_point<system_clock, milliseconds> tp(milliseconds(1500));
  psyassert(ceil<seconds>(tp).time_since_epoch() == seconds(2));
  psyassert(round<seconds>(tp).time_since_epoch() == seconds(2));
}

static void test_duration_converting_constructor() {
  // [time.duration.cons]: the converting constructor is a template over
  // Rep2, not a fixed-Rep parameter -- brace-init from a differently-typed
  // (but convertible) integer must not narrowing-fail at the call site.
  const std::size_t value = 5;
  static_assert(std::is_same_v<decltype(milliseconds{value}), milliseconds>);
  static_assert(milliseconds{value}.count() == 5);
  const int negative_ok = -3;
  static_assert(seconds{negative_ok}.count() == -3);

  // The floating-point guardrail survives: a float can't silently truncate
  // into an integral-rep duration (matching real libstdc++/libc++).
  static_assert(!std::is_constructible_v<milliseconds, double>);
  static_assert(std::is_constructible_v<duration<double>, int>);

  using long_milliseconds = duration<long, std::milli>;
  static_assert(long_milliseconds(milliseconds::min()).count() ==
                std::numeric_limits<long>::min());
  static_assert(long_milliseconds(milliseconds::max()).count() ==
                std::numeric_limits<long>::max());
}

static void test_integral_is_finite() {
  static_assert(std::isfinite(0));
  static_assert(!std::isinf(0LL));
  static_assert(!std::isnan(0U));
}

int main() {
  test_common_type();
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

  test_duration_cast_paths();
  test_duration_cast_wide_intermediate();
  test_duration_fractional_conversion();
  test_time_point_arithmetic();
  test_duration_bounds();
  test_duration_increment_decrement();
  test_chrono_ceil_round_abs();
  test_duration_converting_constructor();
  test_integral_is_finite();
}
