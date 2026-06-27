#include <cstddef>

static_assert(sizeof(std::size_t) == sizeof(void*));
static_assert(sizeof(std::ptrdiff_t) == sizeof(void*));
static_assert(sizeof(std::byte) == 1);

int main() {
  std::size_t n = 42;
  std::ptrdiff_t d = -1;
  std::nullptr_t p = nullptr;
  (void)n;
  (void)d;
  (void)p;

  std::byte b{0b10101010};
  b |= std::byte{0x01};
  b &= std::byte{0xff};
  b ^= std::byte{0x0f};
  b = ~b;
  b <<= 1;
  b >>= 1;
  (void)b;

  auto i = std::to_integer<int>(std::byte{7});
  (void)i;
}
