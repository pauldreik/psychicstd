#include <bit>
#include <cassert>
#include <cmath>
#include <cstdint>

// Quake III's fast inverse square root: bit_cast the float's bit pattern to
// an integer, do the "magic number" trick, bit_cast back. A classic exercise
// of bit_cast round-tripping a value's representation between types.
float fast_inverse_sqrt(float x) {
  float xhalf = 0.5f * x;
  std::uint32_t i = std::bit_cast<std::uint32_t>(x);
  i = 0x5f3759df - (i >> 1);
  float y = std::bit_cast<float>(i);
  y = y * (1.5f - xhalf * y * y); // one Newton iteration
  return y;
}

int main() {
  static_assert(std::bit_cast<std::uint32_t>(0.0f) == 0u);

  float y = fast_inverse_sqrt(4.0f);
  float expected = 1.0f / std::sqrt(4.0f);
  assert(std::fabs(y - expected) < 1e-2f);

  // Round-tripping through bit_cast twice must be the identity.
  std::uint32_t bits = std::bit_cast<std::uint32_t>(3.14f);
  float back = std::bit_cast<float>(bits);
  assert(back == 3.14f);
}
