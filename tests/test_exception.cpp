#include "psyassert.h"
#include <exception>
#include <new>

#if defined(__SANITIZE_ADDRESS__)
#define PSYCHICSTD_TEST_WITH_ASAN 1
#elif defined(__has_feature)
#if __has_feature(address_sanitizer)
#define PSYCHICSTD_TEST_WITH_ASAN 1
#endif
#endif

int main() {
  std::exception e;
  psyassert(e.what() != nullptr);

#ifndef PSYCHICSTD_TEST_WITH_ASAN
  // Regression (fmt util_test.format_system_error): a failed huge allocation
  // makes the C++ runtime's operator new throw ITS std::bad_alloc. The catch
  // must match even though the handler's typeinfo comes from these headers --
  // i.e. our exception classes must share vtable/typeinfo with the runtime.
  auto huge = static_cast<unsigned long>(-1) / 2;
  bool caught_bad_alloc = false;
  try {
    void* p = ::operator new(huge);
    ::operator delete(p);
  } catch (const std::bad_alloc&) {
    caught_bad_alloc = true;
  } catch (...) {
  }
  psyassert(caught_bad_alloc);

  // The same runtime-thrown exception must also match the base class.
  bool caught_as_exception = false;
  try {
    void* p = ::operator new(huge);
    ::operator delete(p);
  } catch (const std::exception&) {
    caught_as_exception = true;
  } catch (...) {
  }
  psyassert(caught_as_exception);
#endif

  // bad_array_new_length is-a bad_alloc.
  bool caught_banl = false;
  try {
    throw std::bad_array_new_length();
  } catch (const std::bad_alloc&) {
    caught_banl = true;
  } catch (...) {
  }
  psyassert(caught_banl);

  std::exception_ptr saved;
  try {
    throw 42;
  } catch (...) {
    saved = std::current_exception();
  }
  psyassert(saved);
  bool caught_saved = false;
  try {
    std::rethrow_exception(saved);
  } catch (int value) {
    caught_saved = value == 42;
  } catch (...) {
  }
  psyassert(caught_saved);
}
