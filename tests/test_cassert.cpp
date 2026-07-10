#include <cassert>

// Binary search that documents its invariants with assert(): the array must
// be sorted, and the returned index (if any) must actually contain the key.
int binary_search(const int* a, int n, int key) {
  int lo = 0, hi = n - 1;
  while (lo <= hi) {
    assert(lo >= 0 && hi < n);
    int mid = lo + (hi - lo) / 2;
    assert(mid >= lo && mid <= hi);
    if (a[mid] == key)
      return mid;
    if (a[mid] < key)
      lo = mid + 1;
    else
      hi = mid - 1;
  }
  return -1;
}

int main() {
  int a[] = {1, 3, 4, 7, 9, 12, 15, 20};
  int n = sizeof(a) / sizeof(a[0]);

  for (int i = 0; i < n; ++i) {
    int idx = binary_search(a, n, a[i]);
    assert(idx == i);
  }
  assert(binary_search(a, n, 6) == -1);
  assert(binary_search(a, n, 0) == -1);
  assert(binary_search(a, n, 21) == -1);
}
