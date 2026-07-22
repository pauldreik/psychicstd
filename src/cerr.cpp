#include "stdio_streambuf.h"

#include <iostream>

namespace {

psychicstd_detail::stdio_streambuf
    __attribute__((init_priority(psychicstd_detail::stream_init_priority)))
    cerr_buffer(stderr);

} // namespace

namespace std {

ostream __attribute__((init_priority(psychicstd_detail::stream_init_priority)))
cerr(&cerr_buffer);

} // namespace std
