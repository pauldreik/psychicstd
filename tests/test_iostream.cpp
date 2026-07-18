#include "psyassert.h"
#include <iomanip>
#include <iostream>
#include <sstream>

class available_buffer : public std::streambuf {
public:
  char* buffer = nullptr;
  std::streamsize size = -1;

private:
  std::streamsize showmanyc() override { return 42; }
  std::streambuf* setbuf(char* value, std::streamsize count) override {
    buffer = value;
    size = count;
    return this;
  }
};

class derived_output : public std::ostream {
public:
  using std::ostream::ostream;
  ~derived_output() override = default;
};

int main() {
  available_buffer available;
  psyassert(available.in_avail() == 42);
  char storage[4];
  psyassert(available.pubsetbuf(storage, 4) == &available);
  psyassert(available.buffer == storage);
  psyassert(available.size == 4);

  std::ostringstream buf;
  auto* old = std::cout.rdbuf(buf.rdbuf());
  std::cout << "test";
  std::cout.rdbuf(old);
  psyassert(buf.str() == "test");

  std::wostringstream wide;
  wide << 42 << L'-' << L"wide";
  psyassert(wide.str() == L"42-wide");

  std::ostringstream point;
  point << std::fixed << std::setprecision(0) << std::showpoint << 3.0;
  psyassert(point.str() == "3.");
}
