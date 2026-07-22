#include "psyassert.h"
#include <complex>

int main() {
  std::complex<double> c(3.0, 4.0);
  psyassert(c.real() == 3.0);
  c *= std::complex<double>(2.0, -1.0);
  psyassert(c == std::complex<double>(10.0, 5.0));
}
