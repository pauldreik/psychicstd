// Module TU: brings in <string> by importing a precompiled header unit.
// Body is identical to tu_include.cpp so the two are directly comparable.
import "pch.h";

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
