#pragma once

// C11 <uchar.h>. Consumers probe for it with __has_include(<uchar.h>) (e.g.
// simdutf's C API header, which otherwise does "#define char16_t uint16_t"
// and breaks C++ keyword char16_t). Glibc ships a real <uchar.h>; chain to it
// with include_next so mbrtoc16/c16rtomb & co stay visible. Darwin's libc has
// none (libc++ papers over that with its own wrapper, which -nostdinc++
// removes), so provide what the header must define: size_t and mbstate_t.
// char16_t/char32_t are C++ keywords and __STDC_UTF_16__/__STDC_UTF_32__ are
// compiler-predefined, so neither needs anything here.
#if defined(__has_include_next) && __has_include_next(<uchar.h>)
#include_next <uchar.h>
#else
#include <stddef.h>
#if __has_include(<sys/_types/_mbstate_t.h>)
#include <sys/_types/_mbstate_t.h> // Darwin
#elif __has_include(<bits/types/mbstate_t.h>)
#include <bits/types/mbstate_t.h>
#endif
#endif
