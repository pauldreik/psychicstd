#include <cassert>
#include <cctype>
#include <cstddef>

// Caesar cipher (ROT13) built entirely from <cctype> primitives: classify
// letters with isalpha/islower/isupper, then shift within their case.
char rot13(char c) {
  if (!std::isalpha(static_cast<unsigned char>(c)))
    return c;
  char base = std::isupper(static_cast<unsigned char>(c)) ? 'A' : 'a';
  return static_cast<char>(base + (c - base + 13) % 26);
}

int main() {
  const char* msg = "Hello, World! 123";
  char rot[32] = {};
  char back[32] = {};
  std::size_t n = 0;
  for (; msg[n]; ++n) {
    rot[n] = rot13(msg[n]);
    back[n] = rot13(rot[n]); // applying ROT13 twice recovers the original
  }
  rot[n] = back[n] = '\0';

  for (std::size_t i = 0; i < n; ++i)
    assert(back[i] == msg[i]);

  assert(rot13('H') == 'U');
  assert(rot13('!') == '!');
  assert(std::toupper('a') == 'A');
  assert(std::tolower('Z') == 'z');
  assert(std::isdigit('7') && !std::isdigit('x'));
  assert(std::isspace(' ') && !std::isspace('x'));
}
