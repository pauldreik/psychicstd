#include <array>
#include <cassert>
#include <span>
#include <vector>

void take_span(std::span<const char> s) { assert(s.size() == 3); }

int main() {
  int arr[3] = {1, 2, 3};
  std::span<int> s1(arr);
  assert(s1.size() == 3);
  assert(s1[1] == 2);

  std::vector<char> v = {'a', 'b', 'c'};
  std::span<char> s2(v);
  assert(s2.data() == v.data());
  take_span(v);

  std::array<char, 3> a = {'x', 'y', 'z'};
  auto s3 = std::span{a};
  assert(s3.size() == 3);

  auto sub = s2.subspan(1);
  assert(sub.size() == 2);
  assert(sub[0] == 'b');
}
