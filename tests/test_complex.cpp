#include "psyassert.h"
#include <complex>

int main() {
  std::complex<double> c(3.0, 4.0);
  psyassert(c.real() == 3.0);
}
