#include "psyassert.h"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>

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
  std::streambuf::pos_type position = 0;
  std::streambuf::off_type offset = 0;
  psyassert(position == offset);

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

  std::wostringstream empty_wide;
  std::swap(wide, empty_wide);
  psyassert(empty_wide.str() == L"42-wide");
  psyassert(wide.str().empty());

  std::ostringstream point;
  point << std::fixed << std::setprecision(0) << std::showpoint << 3.0;
  psyassert(point.str() == "3.");
}
