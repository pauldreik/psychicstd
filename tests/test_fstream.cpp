#include "psyassert.h"
#include <fstream>
#include <string>

int main() {
  std::ofstream out("/tmp/psy_fstream_test.txt");
  out << "hello";
  out.close();
  psyassert(out.is_open() == false);

  std::fstream io("/tmp/psy_fstream_test.txt",
                  std::ios_base::in | std::ios_base::out);
  io.seekp(0, std::ios_base::end);
  io << " world";
  io.seekg(0);
  char text[12]{};
  io.read(text, 11);
  psyassert(text == std::string("hello world"));
}
