#include <cassert>
#include <cstdarg>

// A tiny hand-rolled printf-style summation, built directly on the va_*
// primitives -- the same mechanism std::vprintf and friends are built on.
double sum(int count, ...) {
  std::va_list args;
  va_start(args, count);
  double total = 0;
  for (int i = 0; i < count; ++i)
    total += va_arg(args, double);
  va_end(args);
  return total;
}

double forward_sum(int count, std::va_list args) {
  double total = 0;
  for (int i = 0; i < count; ++i)
    total += va_arg(args, double);
  return total;
}

double sum_via_copy(int count, ...) {
  std::va_list args;
  va_start(args, count);
  std::va_list copy;
  va_copy(copy, args);
  double result = forward_sum(count, copy);
  va_end(copy);
  va_end(args);
  return result;
}

int main() {
  assert(sum(3, 1.0, 2.0, 3.0) == 6.0);
  assert(sum(0) == 0.0);
  assert(sum_via_copy(4, 1.5, 2.5, 3.0, 1.0) == 8.0);
}
