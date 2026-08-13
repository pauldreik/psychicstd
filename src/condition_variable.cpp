#include <condition_variable>
#include <errno.h>

namespace std {

condition_variable::~condition_variable() {
  ::pthread_cond_destroy(&condition_);
}
void condition_variable::notify_one() noexcept {
  ::pthread_cond_signal(&condition_);
}
void condition_variable::notify_all() noexcept {
  ::pthread_cond_broadcast(&condition_);
}
void condition_variable::wait(unique_lock<mutex>& lock) noexcept {
  if (::pthread_cond_wait(&condition_, lock.mutex()->native_handle()) != 0)
    __builtin_trap();
}

cv_status condition_variable::__wait_for_ns(unique_lock<mutex>& lock,
                                            long long ns) noexcept {
  if (ns < 0)
    ns = 0;
  timespec timeout;
  ::clock_gettime(CLOCK_REALTIME, &timeout);
  timeout.tv_sec += ns / 1000000000LL;
  timeout.tv_nsec += ns % 1000000000LL;
  if (timeout.tv_nsec >= 1000000000L) {
    ++timeout.tv_sec;
    timeout.tv_nsec -= 1000000000L;
  }
  int result = ::pthread_cond_timedwait(
      &condition_, lock.mutex()->native_handle(), &timeout);
  if (result == 0)
    return cv_status::no_timeout;
  if (result == ETIMEDOUT)
    return cv_status::timeout;
  __builtin_trap();
}

}
