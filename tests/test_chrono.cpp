#include <cassert>
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
  assert(result.count() == 63'514'168'108'767LL);
}

static void test_duration_cast_identity() {
  milliseconds ms(42);
  auto ns = duration_cast<nanoseconds>(ms);
  assert(ns.count() == 42'000'000);
}

static void test_duration_cast_down() {
  nanoseconds ns(1'500'000'000);
  auto sec = duration_cast<seconds>(ns);
  assert(sec.count() == 1);
}

static void test_duration_cast_up() {
  seconds sec(2);
  auto ms = duration_cast<milliseconds>(sec);
  assert(ms.count() == 2'000);
}

static void test_time_point_arithmetic() {
  time_point<system_clock, milliseconds> epoch(milliseconds(0));
  assert((epoch + milliseconds(5)).time_since_epoch().count() == 5);
  assert((epoch - milliseconds(2)).time_since_epoch().count() == -2);
}

static void test_duration_bounds() {
  assert(seconds::min().count() == std::numeric_limits<long long>::min());
  assert(seconds::max().count() == std::numeric_limits<long long>::max());
}

static void test_integral_is_finite() {
  static_assert(std::isfinite(0));
  static_assert(!std::isinf(0LL));
  static_assert(!std::isnan(0U));
}

int main() {
  test_duration_cast_identity();
  test_duration_cast_down();
  test_duration_cast_up();
  test_duration_cast_same_period();
  test_time_point_arithmetic();
  test_duration_bounds();
  test_integral_is_finite();
}
