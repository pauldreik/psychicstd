#include <cassert>
#include <string_view>

int main() {
  std::string_view sv = "hello";
  assert(sv.size() == 5);
}
