#include <cassert>
#include <cstring>
#include <ctime>

int main() {
  // A known, fixed instant: 2000-01-01 00:00:00 UTC.
  std::time_t epoch_2000 = 946684800;
  std::tm* utc = std::gmtime(&epoch_2000);
  assert(utc != nullptr);
  assert(utc->tm_year == 100); // years since 1900
  assert(utc->tm_mon == 0);
  assert(utc->tm_mday == 1);
  assert(utc->tm_wday == 6); // 2000-01-01 was a Saturday

  char buf[32];
  std::size_t len = std::strftime(buf, sizeof(buf), "%Y-%m-%d", utc);
  assert(len > 0);
  assert(std::strcmp(buf, "2000-01-01") == 0);

  std::time_t later = epoch_2000 + 3600; // one hour later
  assert(std::difftime(later, epoch_2000) == 3600.0);
}
