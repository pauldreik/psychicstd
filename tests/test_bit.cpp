#include "psyassert.h"
#include <bit>
#include <cstdint>

int main() {
  static_assert(std::bit_width(std::uint8_t{0x80}) == 8);
  static_assert(std::bit_width(std::uint32_t{0}) == 0);
  static_assert(std::bit_width(std::uint32_t{1}) == 1);
  static_assert(std::bit_width(std::uint32_t{2}) == 2);
  static_assert(std::bit_width(std::uint32_t{3}) == 2);
  static_assert(std::bit_width(std::uint32_t{0x80000000}) == 32);
  static_assert(std::bit_width(UINT64_MAX) == 64);

  volatile std::uint64_t runtime_value = 0x100;
  psyassert(std::bit_width(runtime_value) == 9);
}
