#include <cassert>
#include <cctype>
#include <ostream>
#include <streambuf>
#include <string>

// A streambuf that uppercases everything written through it, backed by a
// std::string instead of a file or another stream -- the same technique
// used to bridge custom sinks (loggers, in-memory buffers) into iostreams.
class upper_streambuf : public std::streambuf {
public:
  std::string result;

protected:
  int_type overflow(int_type c) override {
    if (traits_type::eq_int_type(c, traits_type::eof()))
      return traits_type::not_eof(c);
    result.push_back(
        static_cast<char>(std::toupper(traits_type::to_char_type(c))));
    return c;
  }

  std::streamsize xsputn(const char* s, std::streamsize n) override {
    for (std::streamsize i = 0; i < n; ++i)
      result.push_back(
          static_cast<char>(std::toupper(static_cast<unsigned char>(s[i]))));
    return n;
  }
};

int main() {
  upper_streambuf buf;
  std::ostream os(&buf);
  os << "hello, " << 42 << " worlds!";
  os.flush();
  assert(buf.result == "HELLO, 42 WORLDS!");
}
