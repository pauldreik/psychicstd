#include <cassert>
#include <cinttypes>
#include <cstdio>
#include <cstring>

int main() {
  std::int64_t big = -123456789012345LL;
  std::uint32_t small = 0xdeadbeefu;

  char buf[64];
  std::snprintf(buf, sizeof(buf), "%" PRId64 " %" PRIx32, big, small);
  assert(std::strcmp(buf, "-123456789012345 deadbeef") == 0);

  std::int64_t roundtrip = 0;
  std::sscanf(buf, "%" SCNd64, &roundtrip);
  assert(roundtrip == big);
}
