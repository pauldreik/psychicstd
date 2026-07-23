#include "stdio_streambuf.h"

namespace psychicstd_detail {

stdio_streambuf::stdio_streambuf(FILE* file) : file_(file) {}

stdio_streambuf::~stdio_streambuf() = default;

std::streamsize stdio_streambuf::xsputn(const char* text,
                                        std::streamsize size) {
  return static_cast<std::streamsize>(
      ::fwrite(text, 1, static_cast<size_t>(size), file_));
}

stdio_streambuf::int_type stdio_streambuf::overflow(int_type value) {
  if (traits_type::eq_int_type(value, traits_type::eof()))
    return traits_type::eof();
  char character = traits_type::to_char_type(value);
  return ::fwrite(&character, 1, 1, file_) == 1 ? value : traits_type::eof();
}

int stdio_streambuf::sync() { return ::fflush(file_) == 0 ? 0 : -1; }

} // namespace psychicstd_detail
