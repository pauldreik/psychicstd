#include "psyassert.h"
#include <exception>
#include <new>

int main() {
  std::exception e;
  psyassert(e.what() != nullptr);

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

  // bad_array_new_length is-a bad_alloc.
  bool caught_banl = false;
  try {
    throw std::bad_array_new_length();
  } catch (const std::bad_alloc&) {
    caught_banl = true;
  } catch (...) {
  }
  psyassert(caught_banl);
}
