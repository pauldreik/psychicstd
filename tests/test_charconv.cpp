#include "psyassert.h"
#include <charconv>
#include <cstdint>
#include <limits>
#include <string_view>
#include <type_traits>

static_assert(std::is_same_v<std::make_unsigned_t<int>, unsigned int>);

template <typename T> void expect_value(std::string_view input, T expected) {
  T value{};
  auto result =
      std::from_chars(input.data(), input.data() + input.size(), value);
  psyassert(result.ptr == input.data() + input.size());
  psyassert(result.ec == std::errc{});
  psyassert(value == expected);
}

int main() {
  expect_value<int>("123", 123);
  expect_value<int>("-42", -42);
  expect_value<int8_t>("-128", -128);
  expect_value<uint8_t>("255", 255);
  expect_value<int>("-2147483648", std::numeric_limits<int>::min());

  int hex = 0;
  const char hex_input[] = "7f!";
  auto hex_result = std::from_chars(hex_input, hex_input + 3, hex, 16);
  psyassert(hex_result.ptr == hex_input + 2);
  psyassert(hex_result.ec == std::errc{});
  psyassert(hex == 127);

  int unchanged = 9;
  const char invalid[] = "+1";
  auto invalid_result = std::from_chars(invalid, invalid + 2, unchanged);
  psyassert(invalid_result.ptr == invalid);
  psyassert(invalid_result.ec == std::errc::invalid_argument);
  psyassert(unchanged == 9);

  const char overflow[] = "2147483648x";
  auto overflow_result = std::from_chars(overflow, overflow + 11, unchanged);
  psyassert(overflow_result.ptr == overflow + 10);
  psyassert(overflow_result.ec == std::errc::result_out_of_range);
  psyassert(unchanged == 9);

#if !defined(__APPLE__) || defined(PSYCHICSTD_TEST_PSYCHICSTD)
  // libc++'s floating-point overloads require macOS 26, while psychicstd
  // supports macOS 14.4 and newer.
  expect_value<float>("1.5", 1.5F);
  expect_value<double>("2.5", 2.5);
  expect_value<long double>("3.5", 3.5L);
  expect_value<double>("-1.25e3", -1250.0);

  float bad_float = 9.0F;
  const char invalid_float[] = "x";
  auto invalid_float_result =
      std::from_chars(invalid_float, invalid_float + 1, bad_float);
  psyassert(invalid_float_result.ptr == invalid_float);
  psyassert(invalid_float_result.ec == std::errc::invalid_argument);
  psyassert(bad_float == 9.0F);

  // from_chars must not require null-termination: [first, last) is a slice
  // of a larger buffer whose next byte is not itself part of the number.
  const char float_slice[] = "1.5x";
  double sliced = 0;
  auto sliced_result = std::from_chars(float_slice, float_slice + 3, sliced);
  psyassert(sliced_result.ptr == float_slice + 3);
  psyassert(sliced_result.ec == std::errc{});
  psyassert(sliced == 1.5);

  const char fixed_input[] = "1.25e2";
  double fixed = 0;
  auto fixed_result = std::from_chars(fixed_input, fixed_input + 6, fixed,
                                      std::chars_format::fixed);
  psyassert(fixed_result.ptr == fixed_input + 4);
  psyassert(fixed == 1.25);

  double scientific = 7;
  auto scientific_result = std::from_chars(
      fixed_input, fixed_input + 4, scientific, std::chars_format::scientific);
  psyassert(scientific_result.ptr == fixed_input);
  psyassert(scientific_result.ec == std::errc::invalid_argument);
  psyassert(scientific == 7);

  const char hex_float_input[] = "1.8p+1";
  double hex_float = 0;
  auto hex_float_result = std::from_chars(hex_float_input, hex_float_input + 6,
                                          hex_float, std::chars_format::hex);
  psyassert(hex_float_result.ptr == hex_float_input + 6);
  psyassert(hex_float_result.ec == std::errc{});
  psyassert(hex_float == 3.0);

  char long_float[96];
  for (size_t i = 0; i < sizeof(long_float); ++i)
    long_float[i] = '0';
  long_float[0] = '1';
  long_float[80] = '.';
  long_float[81] = '5';
  double long_value = 0;
  auto long_result = std::from_chars(long_float, long_float + 82, long_value);
  psyassert(long_result.ptr == long_float + 82);
  psyassert(long_result.ec == std::errc{});
#endif
}
