#include <cassert>
#include <regex>

int main() {
  std::regex re("hello");
  assert(std::regex_match("hello", re));
}
