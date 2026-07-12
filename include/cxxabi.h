#pragma once
#include <cstddef>

namespace __cxxabiv1 {

extern "C" char* __cxa_demangle(const char* mangled_name, char* output_buffer,
                                size_t* length, int* status);

} // namespace __cxxabiv1

namespace abi = __cxxabiv1;
