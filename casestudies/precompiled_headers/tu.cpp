#include "pch.h"

// Exercise std::string so the compiler instantiates the machinery behind it --
// that instantiation work is exactly what a PCH caches. Kept to <string> only
// to keep the psychicstd-vs-libstdc++ comparison fair.
std::string check(const std::string& s) {
  if (s.find("banana") != std::string::npos) {
    return "yummy";
  }
  return ":(";
}

int main() {
  std::string a = "banana bread";
  std::string b = a.substr(0, 6);
  b += " split";
  std::string c = check(a) + check(b);
  return (int)(c.size() + b.compare(a));
}
