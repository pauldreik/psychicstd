#include <Eigen/Core>
#include <cassert>

int main() {
  Eigen::Matrix2d m;
  m << 1, 2, 3, 4;

  assert(m(0, 0) == 1);
  assert(m(0, 1) == 2);
  assert(m(1, 0) == 3);
  assert(m(1, 1) == 4);

  Eigen::Vector2d v(5, 6);
  Eigen::Vector2d r = m * v;
  assert(r(0) == 1 * 5 + 2 * 6);
  assert(r(1) == 3 * 5 + 4 * 6);

  return 0;
}
