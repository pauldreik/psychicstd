#include <string>

namespace std {

#if defined(__cpp_exceptions)
template class basic_string<char>;
#endif

} // namespace std
