/*
 * this is a program that reads a file (given as a command line argument)
 * and makes a histogram of words. it is here as an example for
 * a small program that uses the standard library.
 */
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string_view>
#include <unordered_map>
#include <vector>

int main(int argc, char* argv[]) {

  // key is word, value is count
  std::unordered_map<std::string, unsigned> counts;

  for (int i = 1; i < argc; ++i) {
    std::ifstream file(argv[i]);
    if (!file) {
      std::cerr << "could not open file " << argv[i] << '\n';
      continue;
    }
    std::string word;
    while (file >> word) {
      ++counts[word];
    }
  }

  // print the word list, sorted on count. most frequent word first.
  std::vector<std::pair<std::string_view, unsigned>> sorted;
  sorted.reserve(counts.size());
  for (const auto& [name, count] : counts) {
    sorted.emplace_back(name, count);
  }
  std::sort(sorted.begin(), sorted.end(), [](const auto& p1, const auto& p2) {
    return p1.second > p2.second;
  });

  std::cout << "word\tcount\n";
  for (const auto& [name, count] : sorted) {
    std::cout << name << '\t' << count << '\n';
  }
}
