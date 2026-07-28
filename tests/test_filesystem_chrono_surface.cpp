#include "psyassert.h"
#include <filesystem>

int main() {
  using file_time = std::filesystem::file_time_type;
  file_time value(file_time::duration(42));
  auto system_time = file_time::clock::to_sys(value);
  auto round_trip = file_time::clock::from_sys(system_time);
  psyassert(round_trip == value);
}
