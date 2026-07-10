#include <cassert>
#include <cstdlib>

extern "C" int cmp_int(const void* a, const void* b) {
  int ia = *static_cast<const int*>(a);
  int ib = *static_cast<const int*>(b);
  return (ia > ib) - (ia < ib);
}

int main() {
  int arr[] = {5, 3, 8, 1, 9, 2};
  int n = sizeof(arr) / sizeof(arr[0]);
  std::qsort(arr, n, sizeof(int), cmp_int);
  for (int i = 1; i < n; ++i)
    assert(arr[i - 1] <= arr[i]);

  int key = 8;
  void* found = std::bsearch(&key, arr, n, sizeof(int), cmp_int);
  assert(found != nullptr);
  assert(*static_cast<int*>(found) == 8);

  int missing = 100;
  assert(std::bsearch(&missing, arr, n, sizeof(int), cmp_int) == nullptr);

  std::div_t dv = std::div(17, 5);
  assert(dv.quot == 3 && dv.rem == 2);

  assert(std::atoi("123") == 123);
  assert(std::abs(-42) == 42);
}
