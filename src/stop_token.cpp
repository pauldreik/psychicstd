#include <cstdlib>
#include <new>
#include <stop_token>

namespace std {
namespace __stop_detail {

stop_state::stop_state() { pthread_mutex_init(&mutex_, nullptr); }
stop_state::~stop_state() { pthread_mutex_destroy(&mutex_); }

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

stop_callback_base::stop_callback_base() { pthread_cond_init(&done_, nullptr); }
stop_callback_base::~stop_callback_base() { pthread_cond_destroy(&done_); }

void remove_callback(stop_state* state, stop_callback_base* target) {
  stop_callback_base** link = &state->callbacks_;
  while (*link && *link != target)
    link = &(*link)->next_;
  if (*link == target)
    *link = target->next_;
}

bool stop_state::request_stop() noexcept {
  pthread_mutex_lock(&mutex_);
  if (stop_requested_) {
    pthread_mutex_unlock(&mutex_);
    return false;
  }

  __atomic_store_n(&stop_requested_, true, __ATOMIC_RELEASE);
  stop_callback_base* callbacks = callbacks_;
  callbacks_ = nullptr;
  for (auto* cb = callbacks; cb; cb = cb->next_) {
    cb->registered_ = false;
    cb->running_ = true;
  }
  pthread_mutex_unlock(&mutex_);

  for (auto* cb = callbacks; cb; cb = cb->next_) {
    cb->invoke();
    pthread_mutex_lock(&mutex_);
    cb->running_ = false;
    pthread_cond_broadcast(&cb->done_);
    pthread_mutex_unlock(&mutex_);
  }
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
