#include <cstddef>
#include <cstdint>
#include <numeric>
std::uint64_t f(std::size_t a, std::int32_t b) {
  return std::gcd(a, (std::size_t)b);
}
