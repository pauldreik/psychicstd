#include <pthread.h>
#include <sched.h>
#include <thread>
#include <time.h>
#include <unistd.h>

namespace std {

namespace __thread_detail {

static_assert(sizeof(pthread_t) == sizeof(unsigned long),
              "psychicstd thread handle storage does not fit pthread_t");

bool __create(unsigned long& handle, void* (*start)(void*),
              void* state) noexcept {
  pthread_t native_handle;
  if (::pthread_create(&native_handle, nullptr, start, state) != 0)
    return false;
  __builtin_memcpy(&handle, &native_handle, sizeof(handle));
  return true;
}

void __sleep_for(long long nanoseconds) noexcept {
  struct timespec duration;
  duration.tv_sec = nanoseconds / 1'000'000'000LL;
  duration.tv_nsec = nanoseconds % 1'000'000'000LL;
  ::nanosleep(&duration, nullptr);
}

} // namespace __thread_detail

static pthread_t to_native_handle(unsigned long handle) noexcept {
  pthread_t result;
  __builtin_memcpy(&result, &handle, sizeof(result));
  return result;
}

void* thread::__start(void* pointer) noexcept {
  __state* state = static_cast<__state*>(pointer);
  state->run();
  delete state;
  return nullptr;
}

thread::thread(thread&& other) noexcept
    : handle_(other.handle_), joinable_(other.joinable_) {
  other.joinable_ = false;
}

thread& thread::operator=(thread&& other) noexcept {
  if (this != &other) {
    if (joinable_)
      terminate();
    handle_ = other.handle_;
    joinable_ = other.joinable_;
    other.joinable_ = false;
  }
  return *this;
}

thread::~thread() {
  if (joinable_)
    terminate();
}

void thread::join() {
  if (!joinable_ || ::pthread_join(to_native_handle(handle_), nullptr) != 0)
    __builtin_trap();
  joinable_ = false;
}

void thread::detach() {
  if (!joinable_ || ::pthread_detach(to_native_handle(handle_)) != 0)
    __builtin_trap();
  joinable_ = false;
}

thread::id thread::get_id() const noexcept {
  return joinable_ ? id(handle_) : id();
}

unsigned thread::hardware_concurrency() noexcept {
  long count = ::sysconf(_SC_NPROCESSORS_ONLN);
  return count > 0 ? static_cast<unsigned>(count) : 1;
}

jthread& jthread::operator=(jthread&& other) noexcept {
  if (this != &other) {
    if (joinable()) {
      request_stop();
      join();
    }
    thread_ = static_cast<thread&&>(other.thread_);
    source_ = static_cast<stop_source&&>(other.source_);
  }
  return *this;
}

jthread::~jthread() {
  if (joinable()) {
    request_stop();
    join();
  }
}

namespace this_thread {

thread::id get_id() noexcept {
  unsigned long value;
  pthread_t self = ::pthread_self();
  __builtin_memcpy(&value, &self, sizeof(value));
  return thread::id(value);
}

void yield() noexcept { ::sched_yield(); }

} // namespace this_thread
} // namespace std
