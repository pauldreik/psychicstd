// Representative small program: touches a handful of commonly-used headers
// so the resulting binary's dynamic-linking footprint reflects typical usage,
// not just an empty main().
#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>

int main() {
  std::vector<std::string> words{"psychic", "std", "is", "fast"};
  std::sort(words.begin(), words.end());
  std::map<std::string, int> lengths;
  for (const auto& w : words) {
    lengths[w] = static_cast<int>(w.size());
  }
  auto p = std::make_unique<int>(lengths.size());
  return *p == 0;
}
