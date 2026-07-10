#include <cassert>
#include <cstring>

int main() {
  // memmove must handle overlapping regions correctly; memcpy is not
  // required to. Shift a buffer right by 2 into itself.
  char buf[] = "ABCDEFGH";
  std::memmove(buf + 2, buf, 6);
  assert(std::memcmp(buf, "ABABCDEF", 8) == 0);

  char sentence[] = "the quick brown fox";
  int words = 0;
  char* tok = std::strtok(sentence, " ");
  while (tok) {
    ++words;
    tok = std::strtok(nullptr, " ");
  }
  assert(words == 4);

  assert(std::strcmp("abc", "abd") < 0);
  assert(std::strncmp("abcdef", "abcxyz", 3) == 0);
  assert(std::strlen("hello") == 5);

  const char* haystack = "find the needle here";
  assert(std::strstr(haystack, "needle") != nullptr);
  assert(std::strstr(haystack, "missing") == nullptr);

  char dst[16];
  std::strcpy(dst, "hi");
  std::strcat(dst, " there");
  assert(std::strcmp(dst, "hi there") == 0);
}
