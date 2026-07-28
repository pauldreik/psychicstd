#include "psyassert.h"
#include <bit>
#include <climits>
#include <cstdint>

int main() {
  static_assert(std::endian::native == std::endian::little ||
                std::endian::native == std::endian::big);
  static_assert(std::bit_width(std::uint8_t{0x80}) == 8);
  static_assert(std::bit_width(std::uint32_t{0}) == 0);
  static_assert(std::bit_width(std::uint32_t{1}) == 1);
  static_assert(std::bit_width(std::uint32_t{2}) == 2);
  static_assert(std::bit_width(std::uint32_t{3}) == 2);
  static_assert(std::bit_width(std::uint32_t{0x80000000}) == 32);
  static_assert(std::bit_width(UINT64_MAX) == 64);
  static_assert(std::countr_zero(std::uint8_t{0}) == 8);
  static_assert(std::countr_zero(std::uint32_t{1}) == 0);
  static_assert(std::countr_zero(std::uint32_t{0x100}) == 8);
  static_assert(std::rotl(std::uint8_t{0x81}, 1) == 0x03);
  static_assert(std::rotl(std::uint8_t{0x81}, -1) == 0xc0);
  static_assert(std::rotl(std::uint8_t{0x81}, 9) == 0x03);
  static_assert(std::rotr(std::uint8_t{0x81}, 1) == 0xc0);
  static_assert(std::rotr(std::uint8_t{0x81}, INT_MIN) == 0x81);

  volatile std::uint64_t runtime_value = 0x100;
  psyassert(std::bit_width(runtime_value) == 9);
  psyassert(std::countr_zero(runtime_value) == 8);
}
