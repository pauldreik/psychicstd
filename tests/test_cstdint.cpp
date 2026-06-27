#include <cstdint>

static_assert(sizeof(std::int8_t) == 1);
static_assert(sizeof(std::int16_t) == 2);
static_assert(sizeof(std::int32_t) == 4);
static_assert(sizeof(std::int64_t) == 8);

static_assert(sizeof(std::uint8_t) == 1);
static_assert(sizeof(std::uint16_t) == 2);
static_assert(sizeof(std::uint32_t) == 4);
static_assert(sizeof(std::uint64_t) == 8);

static_assert(sizeof(std::intptr_t) == sizeof(void*));
static_assert(sizeof(std::uintptr_t) == sizeof(void*));

int main() {
  std::int32_t a = -1;
  std::uint64_t b = 0xffffffffffffffff;
  std::intptr_t p = reinterpret_cast<std::intptr_t>(&a);
  (void)a;
  (void)b;
  (void)p;
}
