#include <cassert>
#include <fstream>

int main() {
  std::ofstream out("/tmp/psy_fstream_test.txt");
  out << "hello";
  out.close();
  assert(out.is_open() == false);
}
