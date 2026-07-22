#ifndef __APPLE__

#include "stdio_streambuf.h"

#include <iostream>

namespace {

psychicstd_detail::stdio_streambuf
    __attribute__((init_priority(psychicstd_detail::stream_init_priority)))
    cin_buffer(stdin);

} // namespace

namespace std {

istream __attribute__((init_priority(psychicstd_detail::stream_init_priority)))
cin(&cin_buffer);

} // namespace std

#endif
