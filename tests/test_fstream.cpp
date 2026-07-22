#include "psyassert.h"
#include <cstdio>
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

  const char* seek_path = "/tmp/psy_fstream_seek_test.txt";
  std::remove(seek_path);
  std::filebuf seek;
  psyassert(seek.open(seek_path, std::ios_base::in | std::ios_base::out |
                                     std::ios_base::trunc));
  psyassert(seek.sputn("abcdefghijklmnopqrstuvwxyz", 26) == 26);
  std::streampos pos = seek.pubseekoff(-15, std::ios_base::cur);
  psyassert(pos == std::streampos(11));
  psyassert(seek.sgetc() == 'l');
  psyassert(seek.close());
  std::remove(seek_path);

  const char* wide_seek_path = "/tmp/psy_wfstream_seek_test.txt";
  std::remove(wide_seek_path);
  std::wfilebuf wide_seek;
  psyassert(wide_seek.open(wide_seek_path, std::ios_base::in |
                                               std::ios_base::out |
                                               std::ios_base::trunc));
  psyassert(wide_seek.sputn(L"abcdefghijklmnopqrstuvwxyz", 26) == 26);
  pos = wide_seek.pubseekoff(-15, std::ios_base::cur);
  psyassert(pos == std::streampos(11));
  psyassert(wide_seek.sgetc() == L'l');
  psyassert(wide_seek.close());
  std::remove(wide_seek_path);
}
