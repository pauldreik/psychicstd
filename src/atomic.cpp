#include <atomic>
#include <sched.h>

#if defined(__linux__)
#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>
#elif defined(__APPLE__)
// The CMake integrations require macOS 14.4+, where this API is available.
#include <os/os_sync_wait_on_address.h>
#endif

namespace std::__atomic_detail {

void wait_platform(const volatile void* ptr, const void* old,
                   size_t size) noexcept {
#if defined(__linux__)
  if (size == sizeof(int)) {
    int expected;
    __builtin_memcpy(&expected, old, sizeof(expected));
    ::syscall(SYS_futex, const_cast<void*>(ptr), FUTEX_WAIT_PRIVATE, expected,
              nullptr, nullptr, 0);
  } else {
    ::sched_yield();
  }
#else
  if (size == 4 || size == 8) {
    uint64_t expected = 0;
    __builtin_memcpy(&expected, old, size);
    ::os_sync_wait_on_address(const_cast<void*>(ptr), expected, size,
                              OS_SYNC_WAIT_ON_ADDRESS_NONE);
  } else {
    ::sched_yield();
  }
#endif
}

void notify_platform(const volatile void* ptr, size_t size,
                     int count) noexcept {
#if defined(__linux__)
  if (size == sizeof(int))
    ::syscall(SYS_futex, const_cast<void*>(ptr), FUTEX_WAKE_PRIVATE, count,
              nullptr, nullptr, 0);
#else
  if (size == 4 || size == 8) {
    if (count == 1)
      ::os_sync_wake_by_address_any(const_cast<void*>(ptr), size,
                                    OS_SYNC_WAKE_BY_ADDRESS_NONE);
    else
      ::os_sync_wake_by_address_all(const_cast<void*>(ptr), size,
                                    OS_SYNC_WAKE_BY_ADDRESS_NONE);
  }
#endif
}

}
