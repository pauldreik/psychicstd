#include <sstream>

namespace std {

#if defined(__cpp_exceptions)
template class basic_stringbuf<char, char_traits<char>, allocator<char>>;
template class basic_ostringstream<char, char_traits<char>, allocator<char>>;
template class basic_istringstream<char, char_traits<char>, allocator<char>>;
template class basic_stringstream<char, char_traits<char>, allocator<char>>;
#endif

} // namespace std
