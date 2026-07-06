#include <array>
#include <cassert>
#include <cstddef>
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

  int data[4] = {1, 2, 3, 4};
  auto bytes = std::as_bytes(std::span(data));
  assert(bytes.size() == sizeof(data));
  auto wbytes = std::as_writable_bytes(std::span(data));
  wbytes[0] = std::byte{42};
  assert(reinterpret_cast<unsigned char&>(data[0]) == 42);
}
