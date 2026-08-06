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
}
