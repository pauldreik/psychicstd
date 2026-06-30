#include <cassert>
#include <complex>

int main() {
  std::complex<double> c(3.0, 4.0);
  assert(c.real() == 3.0);
}
