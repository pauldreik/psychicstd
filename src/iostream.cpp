#include <cstdio>
#include <iostream>

namespace {

class stdio_streambuf : public std::streambuf {
public:
  explicit stdio_streambuf(FILE* file) : file_(file) {}

protected:
  std::streamsize xsputn(const char* text, std::streamsize size) override {
    return static_cast<std::streamsize>(
        ::fwrite(text, 1, static_cast<size_t>(size), file_));
  }

  int_type overflow(int_type value) override {
    if (traits_type::eq_int_type(value, traits_type::eof()))
      return traits_type::eof();
    char character = traits_type::to_char_type(value);
    return ::fwrite(&character, 1, 1, file_) == 1 ? value : traits_type::eof();
  }

  int sync() override { return ::fflush(file_) == 0 ? 0 : -1; }

private:
  FILE* file_;
};

// Construct before ordinary user globals and destroy after them. psychicstd is
// Linux-only, and both supported compilers implement init_priority.
constexpr int stream_init_priority = 101;
stdio_streambuf
    __attribute__((init_priority(stream_init_priority))) cout_buffer(stdout);
stdio_streambuf
    __attribute__((init_priority(stream_init_priority))) cerr_buffer(stderr);
stdio_streambuf
    __attribute__((init_priority(stream_init_priority))) clog_buffer(stderr);
stdio_streambuf
    __attribute__((init_priority(stream_init_priority))) cin_buffer(stdin);

} // namespace

namespace std {

ios_base::failure::failure(const string& message)
    : system_error(error_code(1, generic_category()), message) {}

ios_base::failure::failure(const char* message)
    : system_error(error_code(1, generic_category()), message) {}

ios_base::failure::failure(const string& message, const error_code& code)
    : system_error(code, message) {}

ios_base::failure::failure(const char* message, const error_code& code)
    : system_error(code, message) {}

void ios_base::_throw_failure_with_exceptions() {
  _PSYCHICSTD_THROW(failure("basic_ios::clear"));
}

template class basic_ostream<char, char_traits<char>>;
template class basic_istream<char, char_traits<char>>;

ostream __attribute__((init_priority(stream_init_priority))) cout(&cout_buffer);
ostream __attribute__((init_priority(stream_init_priority))) cerr(&cerr_buffer);
ostream __attribute__((init_priority(stream_init_priority))) clog(&clog_buffer);
istream __attribute__((init_priority(stream_init_priority))) cin(&cin_buffer);

} // namespace std
