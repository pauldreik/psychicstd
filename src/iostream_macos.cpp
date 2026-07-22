#ifdef __APPLE__

#include "stdio_streambuf.h"

#include <istream>
#include <new>
#include <ostream>

namespace std {

// These arrays provide the linker symbols declared as streams by <iostream>.
// ios_base::Init starts the object lifetimes before a user global can use them.
alignas(istream) unsigned char cin[sizeof(istream)];
alignas(ostream) unsigned char cout[sizeof(ostream)];
alignas(ostream) unsigned char cerr[sizeof(ostream)];
alignas(ostream) unsigned char clog[sizeof(ostream)];

} // namespace std

namespace {

alignas(psychicstd_detail::stdio_streambuf) unsigned char cin_buffer_storage
    [sizeof(psychicstd_detail::stdio_streambuf)];
alignas(psychicstd_detail::stdio_streambuf) unsigned char cout_buffer_storage
    [sizeof(psychicstd_detail::stdio_streambuf)];
alignas(psychicstd_detail::stdio_streambuf) unsigned char cerr_buffer_storage
    [sizeof(psychicstd_detail::stdio_streambuf)];
alignas(psychicstd_detail::stdio_streambuf) unsigned char clog_buffer_storage
    [sizeof(psychicstd_detail::stdio_streambuf)];

bool streams_initialized = false;

void initialize_standard_streams() {
  if (streams_initialized)
    return;

  auto* cin_buffer =
      ::new (cin_buffer_storage) psychicstd_detail::stdio_streambuf(stdin);
  auto* cout_buffer =
      ::new (cout_buffer_storage) psychicstd_detail::stdio_streambuf(stdout);
  auto* cerr_buffer =
      ::new (cerr_buffer_storage) psychicstd_detail::stdio_streambuf(stderr);
  auto* clog_buffer =
      ::new (clog_buffer_storage) psychicstd_detail::stdio_streambuf(stderr);

  ::new (std::cin) std::istream(cin_buffer);
  ::new (std::cout) std::ostream(cout_buffer);
  ::new (std::cerr) std::ostream(cerr_buffer);
  ::new (std::clog) std::ostream(clog_buffer);
  streams_initialized = true;
}

} // namespace

namespace std {

ios_base::Init::Init() { initialize_standard_streams(); }
ios_base::Init::~Init() = default;

} // namespace std

#endif
