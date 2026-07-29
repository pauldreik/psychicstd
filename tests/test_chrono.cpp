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

static void test_time_point_arithmetic() {
  time_point<system_clock, milliseconds> epoch(milliseconds(0));
  psyassert((epoch + milliseconds(5)).time_since_epoch().count() == 5);
  psyassert((epoch - milliseconds(2)).time_since_epoch().count() == -2);
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

static void test_integral_is_finite() {
  static_assert(std::isfinite(0));
  static_assert(!std::isinf(0LL));
  static_assert(!std::isnan(0U));
}

int main() {
  using namespace std::chrono_literals;
  static_assert(2h == std::chrono::hours(2));
  static_assert(3min == std::chrono::minutes(3));
  static_assert(4s == std::chrono::seconds(4));
  static_assert(5ms == std::chrono::milliseconds(5));
  static_assert(6us == std::chrono::microseconds(6));
  static_assert(7ns == std::chrono::nanoseconds(7));
  static_assert(10ms / 2ms == 5);
  static_assert(2s / 500ms == 4);

  test_duration_cast_identity();
  test_duration_cast_down();
  test_duration_cast_up();
  test_duration_cast_wide_intermediate();
  test_duration_fractional_conversion();
  test_duration_cast_same_period();
  test_time_point_arithmetic();
  test_duration_bounds();
  test_integral_is_finite();
}
