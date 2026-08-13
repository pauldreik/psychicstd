#include <typeinfo>

namespace std {

#if defined(__APPLE__) && defined(__aarch64__)
namespace {

// Apple arm64 libc++abi uses bit 63 to flag non-unique RTTI names.
const char* masked(const char* name) noexcept {
  return reinterpret_cast<const char*>(reinterpret_cast<size_t>(name) &
                                       ~(size_t(1) << 63));
}

bool unique(const char* name) noexcept {
  return !(reinterpret_cast<size_t>(name) >> 63);
}

}
#endif

bool type_info::__equal(const type_info& rhs) const noexcept {
#if defined(__APPLE__) && defined(__aarch64__)
  return !unique(__name) && !unique(rhs.__name) &&
         __builtin_strcmp(masked(__name), masked(rhs.__name)) == 0;
#else
  return __name[0] != '*' && rhs.__name[0] != '*' &&
         __builtin_strcmp(__name, rhs.__name) == 0;
#endif
}

bool type_info::before(const type_info& rhs) const noexcept {
#if defined(__APPLE__) && defined(__aarch64__)
  if (!unique(__name) && !unique(rhs.__name))
    return __builtin_strcmp(masked(__name), masked(rhs.__name)) < 0;
#else
  if (__name[0] != '*' || rhs.__name[0] != '*')
    return __builtin_strcmp(__name, rhs.__name) < 0;
#endif
  return __name < rhs.__name;
}

size_t type_info::hash_code() const noexcept {
#if defined(__APPLE__) && defined(__aarch64__)
  if (unique(__name))
    return reinterpret_cast<size_t>(__name);
  const char* name = masked(__name);
#else
  if (__name[0] == '*')
    return reinterpret_cast<size_t>(__name);
  const char* name = __name;
#endif
  size_t hash = 14695981039346656037ull;
  for (const char* p = name; *p; ++p)
    hash = (hash ^ static_cast<unsigned char>(*p)) * 1099511628211ull;
  return hash;
}

}
