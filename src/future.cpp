#include <future>
#include <stdexcept>

namespace std::__future_detail {

[[noreturn]] void __throw(const char* message) {
  _PSYCHICSTD_THROW(runtime_error(message));
}

void __state_base::retain() noexcept { __refcount_detail::add(&references); }

bool __state_base::release_reference() noexcept {
  return __refcount_detail::release(&references);
}

void __state_base::set_exception(exception_ptr exception) {
  {
    lock_guard lock(mutex_);
    if (ready)
      __throw("promise already satisfied");
    exception_ = exception;
    ready = true;
  }
  condition_.notify_all();
}

void __state_base::abandon() noexcept {
  unique_lock lock(mutex_);
  if (ready)
    return;
  error_ = __error::broken_promise;
  ready = true;
  lock.unlock();
  condition_.notify_all();
}

void __state_base::retrieve_future() {
  lock_guard lock(mutex_);
  if (future_retrieved)
    __throw("future already retrieved");
  future_retrieved = true;
}

bool __state_base::is_ready() {
  lock_guard lock(mutex_);
  return ready;
}

void __state_base::wait() {
  unique_lock lock(mutex_);
  condition_.wait(lock, [this] { return ready; });
}

void __state<void>::release() noexcept {
  if (release_reference())
    delete this;
}

void __state<void>::set_value() {
  {
    lock_guard lock(mutex_);
    if (ready)
      __throw("promise already satisfied");
    ready = true;
  }
  condition_.notify_all();
}

}

namespace std {

future<void>::future(state* state) noexcept : state_(state) {}

future<void>::future(future&& other) noexcept : state_(other.state_) {
  other.state_ = nullptr;
}

future<void>& future<void>::operator=(future&& other) noexcept {
  if (this != &other) {
    if (state_)
      state_->release();
    state_ = other.state_;
    other.state_ = nullptr;
  }
  return *this;
}

future<void>::~future() {
  if (state_)
    state_->release();
}

void future<void>::wait() const {
  if (!state_)
    __future_detail::__throw("future has no state");
  state_->wait();
}

void future<void>::get() {
  wait();
  if (state_->error_ == __future_detail::__error::broken_promise) {
    state_->release();
    state_ = nullptr;
    __future_detail::__throw("broken promise");
  }
  if (state_->exception_) {
    exception_ptr exception = state_->exception_;
    state_->release();
    state_ = nullptr;
    rethrow_exception(exception);
  }
  state_->release();
  state_ = nullptr;
}

promise<void>::promise() : state_(new state) {}

promise<void>::promise(promise&& other) noexcept : state_(other.state_) {
  other.state_ = nullptr;
}

promise<void>& promise<void>::operator=(promise&& other) noexcept {
  if (this != &other) {
    if (state_)
      state_->abandon();
    if (state_)
      state_->release();
    state_ = other.state_;
    other.state_ = nullptr;
  }
  return *this;
}

promise<void>::~promise() {
  if (state_) {
    state_->abandon();
    state_->release();
  }
}

bool promise<void>::__is_ready() {
  if (!state_)
    __future_detail::__throw("promise has no state");
  return state_->is_ready();
}

future<void> promise<void>::get_future() {
  if (!state_)
    __future_detail::__throw("promise has no state");
  state_->retrieve_future();
  state_->retain();
  return future<void>(state_);
}

void promise<void>::set_value() {
  if (!state_)
    __future_detail::__throw("promise has no state");
  state_->set_value();
}

void promise<void>::set_exception(exception_ptr exception) {
  if (!state_)
    __future_detail::__throw("promise has no state");
  state_->set_exception(exception);
}

}
