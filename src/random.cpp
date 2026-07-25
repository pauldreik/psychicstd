#include <random>

#ifdef __APPLE__
extern "C" int open(const char*, int, ...) __asm("_open");
extern "C" long read(int, void*, unsigned long) __asm("_read");
extern "C" int close(int) __asm("_close");
#define _PSYCHICSTD_O_RDONLY 0x0000
#define _PSYCHICSTD_O_CLOEXEC 0x01000000
#else
#include <fcntl.h>
#include <unistd.h>
#define _PSYCHICSTD_O_RDONLY O_RDONLY
#define _PSYCHICSTD_O_CLOEXEC O_CLOEXEC
#endif

namespace std {

random_device::random_device(const char*) {}

random_device::result_type random_device::operator()() {
  result_type value;
  int fd =
      ::open("/dev/urandom", _PSYCHICSTD_O_RDONLY | _PSYCHICSTD_O_CLOEXEC);
  if (fd >= 0) {
    long count = ::read(fd, &value, sizeof(value));
    ::close(fd);
    if (count == sizeof(value))
      return value;
  }

  static unsigned long long state =
      reinterpret_cast<unsigned long long>(&state);
  unsigned long long old = __atomic_load_n(&state, __ATOMIC_RELAXED);
  unsigned long long next;
  do {
    next = old * 6364136223846793005ULL + 1442695040888963407ULL;
  } while (!__atomic_compare_exchange_n(&state, &old, next, true,
                                        __ATOMIC_RELAXED, __ATOMIC_RELAXED));
  return static_cast<result_type>(next >> 33);
}

double random_device::entropy() const noexcept { return 32.0; }

} // namespace std

#undef _PSYCHICSTD_O_RDONLY
#undef _PSYCHICSTD_O_CLOEXEC
