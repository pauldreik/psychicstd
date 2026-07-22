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

// Construct before ordinary user globals and destroy after them. psychicstd is
// Linux-only, and both supported compilers implement init_priority.
inline constexpr int stream_init_priority = 101;

} // namespace psychicstd_detail
