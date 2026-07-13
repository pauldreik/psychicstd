#include "psyassert.h"
#include <fstream>

int main() {
  std::ofstream out("/tmp/psy_fstream_test.txt");
  out << "hello";
  out.close();
  psyassert(out.is_open() == false);
}
