#include "psyassert.h"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>

#if defined(PSYCHICSTD_TEST_PSYCHICSTD)
#include "../src/stdio_streambuf.h"
#endif

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

class syncing_buffer : public std::stringbuf {
public:
  int sync_calls = 0;

private:
  int sync() override {
    ++sync_calls;
    return std::stringbuf::sync();
  }
};

bool iostream_was_ready_during_static_initialization();
std::ostream* iostream_cout_from_other_translation_unit();

std::ios& set_logging_defaults(std::ios& stream) {
  stream.setf(std::ios::showbase | std::ios::boolalpha | std::ios::internal);
  return stream;
}

int main() {
  psyassert(iostream_was_ready_during_static_initialization());
  psyassert(iostream_cout_from_other_translation_unit() == &std::cout);

#if defined(PSYCHICSTD_TEST_PSYCHICSTD)
  FILE* input_file = std::tmpfile();
  psyassert(input_file);
  psyassert(std::fwrite("stream input", 1, 12, input_file) == 12);
  std::rewind(input_file);
  psychicstd_detail::stdio_streambuf input_buffer(input_file);
  std::istream file_input(&input_buffer);
  psyassert(file_input.peek() == 's');
  char input_text[13] = {};
  file_input.read(input_text, 12);
  psyassert(file_input.gcount() == 12);
  psyassert(std::string(input_text) == "stream input");
  std::fclose(input_file);
#endif

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

  std::stringbuf bidirectional_buffer;
  std::iostream bidirectional(&bidirectional_buffer);
  static_assert(
      std::is_same_v<std::iostream::traits_type, std::char_traits<char>>);
  psyassert(bidirectional.rdbuf() == &bidirectional_buffer);
  bidirectional.setf(std::ios_base::unitbuf);
  psyassert(bidirectional.flags() & std::ios_base::unitbuf);

  syncing_buffer synced;
  std::ostream unit_buffered(&synced);
  unit_buffered << std::unitbuf << "first";
  psyassert(synced.sync_calls == 1);
  unit_buffered << std::nounitbuf << "second";
  psyassert(synced.sync_calls == 1);

  syncing_buffer tied_synced;
  std::ostream tied_stream(&tied_synced);
  syncing_buffer sentry_synced;
  std::ostream sentry_stream(&sentry_synced);
  sentry_stream.tie(&tied_stream);
  {
    std::ostream::sentry sentry(sentry_stream);
    psyassert(sentry);
    psyassert(tied_synced.sync_calls == 1);
  }
  std::ostream failed_sentry(nullptr);
  failed_sentry.tie(&tied_stream);
  {
    std::ostream::sentry sentry(failed_sentry);
    psyassert(!sentry);
    psyassert(tied_synced.sync_calls == 1);
  }
  sentry_stream.tie(nullptr);
  {
    std::ostream::sentry sentry(sentry_stream);
    std::unitbuf(sentry_stream);
  }
  psyassert(sentry_synced.sync_calls == 1);
  try {
    std::ostream::sentry sentry(sentry_stream);
    throw 1;
  } catch (...) {
  }
  psyassert(sentry_synced.sync_calls == 1);

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

  std::ostringstream zero;
  zero << std::showbase << std::hex << std::setfill('_') << std::setw(6) << 0;
  psyassert(zero.str() == "_____0");

  std::ostringstream formatting;
  formatting << std::showpos << 77 << ' ' << std::uppercase << std::scientific
             << 77.0 << ' ' << std::nouppercase << std::hexfloat << 77.0;
  psyassert(formatting.str() == "+77 +7.700000E+01 +0x1.34p+6");

  std::ostringstream internal;
  internal << std::internal << std::setfill('_') << std::setw(6) << -42;
  psyassert(internal.str() == "-___42");

  std::ostringstream ios_manipulator;
  ios_manipulator << set_logging_defaults << true;
  psyassert(ios_manipulator.str() == "true");

  std::istringstream booleans("true false");
  bool first = false;
  bool second = true;
  booleans >> std::boolalpha >> first >> second;
  psyassert(first == true && second == false);

  std::istringstream numeric_booleans("0 1");
  numeric_booleans >> first >> second;
  psyassert(!numeric_booleans.fail());
  psyassert(first == false && second == true);

  std::istringstream invalid_boolean("2");
  bool invalid = false;
  invalid_boolean >> invalid;
  psyassert(invalid);
  psyassert(invalid_boolean.fail());
}
