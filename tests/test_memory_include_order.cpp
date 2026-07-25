#include "psyassert.h"
#include <memory>
#include <vector>

int main() {
  std::vector<int> values{1, 2, 3};
  std::unique_ptr<int> value(new int(4));
  psyassert(values.size() == 3);
  psyassert(*value == 4);
}
