#include <cstdlib>
#include <new>
#include <pthread.h>
#include <stop_token>

namespace std {
namespace __stop_detail {

static_assert(sizeof(pthread_mutex_t) <= sizeof(stop_state::mutex_));
static_assert(alignof(pthread_mutex_t) <= alignof(stop_state));
static_assert(sizeof(pthread_cond_t) <= sizeof(stop_callback_base::done_));
static_assert(alignof(pthread_cond_t) <= alignof(stop_callback_base));

static pthread_mutex_t* mutex(stop_state* state) {
  return reinterpret_cast<pthread_mutex_t*>(state->mutex_);
}
static pthread_cond_t* condition(stop_callback_base* callback) {
  return reinterpret_cast<pthread_cond_t*>(callback->done_);
}

struct callback_frame {
  stop_callback_base* callback;
  callback_frame* previous;
  bool destroyed = false;
};
static thread_local callback_frame* current_callback;

stop_state::stop_state() { pthread_mutex_init(mutex(this), nullptr); }
stop_state::~stop_state() { pthread_mutex_destroy(mutex(this)); }

void stop_state::add_ref() noexcept { __refcount_detail::add(&refs_); }
void stop_state::add_source() noexcept { __refcount_detail::add(&sources_); }
void stop_state::release_source() noexcept {
  __atomic_sub_fetch(&sources_, 1, __ATOMIC_RELEASE);
}
bool stop_state::stop_possible() const noexcept {
  return __atomic_load_n(&stop_requested_, __ATOMIC_ACQUIRE) ||
         __atomic_load_n(&sources_, __ATOMIC_ACQUIRE) != 0;
}
void stop_state::release() noexcept {
  if (__refcount_detail::release(&refs_)) {
    this->~stop_state();
    ::free(this);
  }
}

stop_callback_base::stop_callback_base() {
  pthread_cond_init(condition(this), nullptr);
}
stop_callback_base::~stop_callback_base() {
  pthread_cond_destroy(condition(this));
}

void lock(stop_state* state) noexcept { pthread_mutex_lock(mutex(state)); }
void unlock(stop_state* state) noexcept { pthread_mutex_unlock(mutex(state)); }
void wait(stop_callback_base* callback, stop_state* state) noexcept {
  pthread_cond_wait(condition(callback), mutex(state));
}

void remove_callback(stop_state* state, stop_callback_base* target) {
  stop_callback_base** link = &state->callbacks_;
  while (*link && *link != target)
    link = &(*link)->next_;
  if (*link == target)
    *link = target->next_;
}

bool mark_destroyed_if_current(stop_callback_base* target) noexcept {
  for (auto* frame = current_callback; frame; frame = frame->previous) {
    if (frame->callback == target) {
      frame->destroyed = true;
      return true;
    }
  }
  return false;
}

bool stop_state::request_stop() noexcept {
  lock(this);
  if (stop_requested_) {
    unlock(this);
    return false;
  }

  __atomic_store_n(&stop_requested_, true, __ATOMIC_RELEASE);
  while (callbacks_) {
    stop_callback_base* cb = callbacks_;
    callbacks_ = cb->next_;
    cb->registered_ = false;
    cb->running_ = true;
    unlock(this);

    callback_frame frame{cb, current_callback};
    current_callback = &frame;
    cb->invoke();
    current_callback = frame.previous;

    lock(this);
    if (!frame.destroyed) {
      cb->running_ = false;
      pthread_cond_broadcast(condition(cb));
    }
  }
  unlock(this);
  return true;
}

} // namespace __stop_detail

stop_token::stop_token(const stop_token& other) noexcept
    : state_(other.state_) {
  if (state_)
    state_->add_ref();
}
stop_token::stop_token(stop_token&& other) noexcept : state_(other.state_) {
  other.state_ = nullptr;
}
stop_token::~stop_token() {
  if (state_)
    state_->release();
}
stop_token& stop_token::operator=(const stop_token& other) noexcept {
  if (this != &other) {
    if (state_)
      state_->release();
    state_ = other.state_;
    if (state_)
      state_->add_ref();
  }
  return *this;
}
stop_token& stop_token::operator=(stop_token&& other) noexcept {
  if (this != &other) {
    if (state_)
      state_->release();
    state_ = other.state_;
    other.state_ = nullptr;
  }
  return *this;
}
bool stop_token::stop_possible() const noexcept {
  return state_ && state_->stop_possible();
}
bool stop_token::stop_requested() const noexcept {
  return state_ && __atomic_load_n(&state_->stop_requested_, __ATOMIC_ACQUIRE);
}

stop_source::stop_source() noexcept {
  void* mem = ::malloc(sizeof(__stop_detail::stop_state));
  if (mem)
    state_ = ::new (mem) __stop_detail::stop_state;
}
stop_source::stop_source(const stop_source& other) noexcept
    : state_(other.state_) {
  if (state_) {
    state_->add_ref();
    state_->add_source();
  }
}
stop_source::stop_source(stop_source&& other) noexcept : state_(other.state_) {
  other.state_ = nullptr;
}
stop_source::~stop_source() {
  if (state_) {
    state_->release_source();
    state_->release();
  }
}
stop_source& stop_source::operator=(const stop_source& other) noexcept {
  if (this != &other) {
    if (state_) {
      state_->release_source();
      state_->release();
    }
    state_ = other.state_;
    if (state_) {
      state_->add_ref();
      state_->add_source();
    }
  }
  return *this;
}
stop_source& stop_source::operator=(stop_source&& other) noexcept {
  if (this != &other) {
    if (state_) {
      state_->release_source();
      state_->release();
    }
    state_ = other.state_;
    other.state_ = nullptr;
  }
  return *this;
}
bool stop_source::stop_possible() const noexcept { return state_ != nullptr; }
bool stop_source::stop_requested() const noexcept {
  return state_ && __atomic_load_n(&state_->stop_requested_, __ATOMIC_ACQUIRE);
}
bool stop_source::request_stop() noexcept {
  return state_ && state_->request_stop();
}
stop_token stop_source::get_token() const noexcept {
  return stop_token(state_);
}

} // namespace std
