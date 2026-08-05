#include "psyassert.h"
#include <cstdio>
#include <fstream>
#include <string>
#include <unistd.h>
#include <utility>

int main() {
  const std::string path_prefix =
      "/tmp/psy_fstream_test_" + std::to_string(::getpid());

  std::filebuf unopened_buffer;
  psyassert(unopened_buffer.close() == nullptr);
  std::ofstream unopened_stream;
  unopened_stream.close();
  psyassert(unopened_stream.fail());

  const std::string basic_path = path_prefix + "_basic.txt";
  std::ofstream out(basic_path);
  out << "hello";
  out.close();
  psyassert(out.is_open() == false);

  std::fstream io(basic_path, std::ios_base::in | std::ios_base::out);
  io.seekp(0, std::ios_base::end);
  io << " world";
  io.seekg(0);
  char text[12]{};
  io.read(text, 11);
  psyassert(text == std::string("hello world"));

  std::ifstream moved;
  moved = std::ifstream(basic_path);
  char first{};
  moved.get(first);
  psyassert(first == 'h');

  const std::string move_path = path_prefix + "_move.txt";
  std::remove(move_path.c_str());
  {
    std::ofstream source(move_path);
    std::ofstream destination;
    destination = std::move(source);
    destination << "assigned";
  }
  {
    std::ifstream verify(move_path);
    std::string value;
    verify >> value;
    psyassert(value == "assigned");
  }
  {
    std::fstream source(move_path, std::ios_base::in | std::ios_base::out |
                                       std::ios_base::trunc);
    std::fstream destination(std::move(source));
    destination << "moved";
  }
  {
    std::fstream source(move_path, std::ios_base::in | std::ios_base::out |
                                       std::ios_base::trunc);
    std::fstream destination;
    destination = std::move(source);
    destination << "again";
  }
  {
    std::ifstream verify(move_path);
    std::string value;
    verify >> value;
    psyassert(value == "again");
  }
  std::remove(move_path.c_str());

  const std::string seek_path = path_prefix + "_seek.txt";
  std::remove(seek_path.c_str());
  std::filebuf seek;
  psyassert(seek.open(seek_path.c_str(), std::ios_base::in |
                                             std::ios_base::out |
                                             std::ios_base::trunc));
  psyassert(seek.sputn("abcdefghijklmnopqrstuvwxyz", 26) == 26);
  std::streampos pos = seek.pubseekoff(-15, std::ios_base::cur);
  psyassert(pos == std::streampos(11));
  psyassert(seek.sgetc() == 'l');
  psyassert(seek.close());
  std::remove(seek_path.c_str());

  const std::string wide_seek_path = path_prefix + "_wide_seek.txt";
  std::remove(wide_seek_path.c_str());
  std::wfilebuf wide_seek;
  psyassert(wide_seek.open(wide_seek_path.c_str(), std::ios_base::in |
                                                       std::ios_base::out |
                                                       std::ios_base::trunc));
  psyassert(wide_seek.sputn(L"abcdefghijklmnopqrstuvwxyz", 26) == 26);
  pos = wide_seek.pubseekoff(-15, std::ios_base::cur);
  psyassert(pos == std::streampos(11));
  psyassert(wide_seek.sgetc() == L'l');
  psyassert(wide_seek.close());
  std::remove(wide_seek_path.c_str());
  std::remove(basic_path.c_str());
}
