#include <cassert>
#include <cstdio>
#include <cstring>

int main() {
  char buf[64];
  int n = std::snprintf(buf, sizeof(buf), "%d-%s-%.2f", 42, "hi", 3.14159);
  assert(n > 0);
  assert(std::strcmp(buf, "42-hi-3.14") == 0);

  int a;
  char word[16];
  double d;
  int matched = std::sscanf(buf, "%d-%15[^-]-%lf", &a, word, &d);
  assert(matched == 3);
  assert(a == 42);
  assert(std::strcmp(word, "hi") == 0);
  assert(d > 3.13 && d < 3.15);

  std::FILE* f = std::tmpfile();
  assert(f != nullptr);
  const char* text = "round trip through a real file\n";
  assert(std::fputs(text, f) >= 0);
  std::rewind(f);
  char readback[64] = {};
  assert(std::fgets(readback, sizeof(readback), f) != nullptr);
  assert(std::strcmp(readback, text) == 0);
  std::fclose(f);
}
