#include <string>

std::string check(const std::string& s) {
  if (s.find("banana") != std::string::npos) {
    return "yummy";
  }
  return ":(";
}
