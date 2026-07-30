#include "psyassert.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <span>
#include <vector>

void take_span(std::span<const char> s) { psyassert(s.size() == 3); }

int main() {
  int arr[3] = {1, 2, 3};
  std::span<int> s1(arr);
  psyassert(s1.size() == 3);
  psyassert(s1[1] == 2);

  std::vector<char> v = {'a', 'b', 'c'};
  std::span<char> s2(v);
  psyassert(s2.data() == v.data());
  take_span(v);

  std::array<char, 3> a = {'x', 'y', 'z'};
  auto s3 = std::span{a};
  psyassert(s3.size() == 3);

  auto sub = s2.subspan(1);
  psyassert(sub.size() == 2);
  psyassert(sub[0] == 'b');
  static_assert(decltype(s2.first<2>())::extent == 2);
  psyassert(s2.first<2>()[1] == 'b');
  psyassert(s2.last<2>()[0] == 'b');
  psyassert((s2.subspan<1, 1>()[0] == 'b'));
  std::array<char, 3> same = {'a', 'b', 'c'};
  psyassert(std::ranges::equal(s2, same));

  int data[4] = {1, 2, 3, 4};
  auto bytes = std::as_bytes(std::span(data));
  psyassert(bytes.size() == sizeof(data));
  auto wbytes = std::as_writable_bytes(std::span(data));
  wbytes[0] = std::byte{42};
  psyassert(reinterpret_cast<unsigned char&>(data[0]) == 42);
}
