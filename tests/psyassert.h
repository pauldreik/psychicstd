#pragma once

extern "C" long write(int, const void*, unsigned long);

namespace psyassert_detail {

inline void write_stderr(const char* text) noexcept {
  write(2, text, __builtin_strlen(text));
}

[[noreturn]] inline void fail(const char* expr, const char* location) noexcept {
  write_stderr(location);
  write_stderr(": psyassert failed: ");
  write_stderr(expr);
  write_stderr("\n");
  __builtin_trap();
}

} // namespace psyassert_detail

#define psyassert_stringify_1(x) #x
#define psyassert_stringify(x) psyassert_stringify_1(x)

#define psyassert(expr)                                                        \
  do {                                                                         \
    if (!(expr))                                                               \
      psyassert_detail::fail(#expr,                                            \
                             __FILE__ ":" psyassert_stringify(__LINE__));      \
  } while (0)
