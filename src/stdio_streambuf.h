#pragma once

#include <cstdio>
#include <streambuf>

namespace psychicstd_detail {

class stdio_streambuf : public std::streambuf {
public:
  explicit stdio_streambuf(FILE* file);
  ~stdio_streambuf() override;

protected:
  std::streamsize xsputn(const char* text, std::streamsize size) override;
  int_type overflow(int_type value) override;
  int sync() override;

private:
  FILE* file_;
};

// ELF platforms use this to construct streams before ordinary user globals.
// Mach-O needs the ios_base::Init guard because its ordering is TU-local.
inline constexpr int stream_init_priority = 101;

} // namespace psychicstd_detail
