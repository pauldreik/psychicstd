#include <ostream>

namespace std {

int ios_base::_format_signed(char* buf, size_t size, long long value,
                             unsigned long long unsigned_value) const {
  if (flags_ & hex)
    return __builtin_snprintf(buf, size,
                              (flags_ & showbase) && value
                                  ? ((flags_ & uppercase) ? "0X%llX" : "0x%llx")
                                  : ((flags_ & uppercase) ? "%llX" : "%llx"),
                              unsigned_value);
  if (flags_ & oct)
    return __builtin_snprintf(buf, size,
                              (flags_ & showbase) && value ? "0%llo" : "%llo",
                              unsigned_value);
  return __builtin_snprintf(buf, size, flags_ & showpos ? "%+lld" : "%lld",
                            value);
}

int ios_base::_format_unsigned(char* buf, size_t size,
                               unsigned long long value) const {
  if (flags_ & hex)
    return __builtin_snprintf(buf, size,
                              (flags_ & showbase) && value
                                  ? ((flags_ & uppercase) ? "0X%llX" : "0x%llx")
                                  : ((flags_ & uppercase) ? "%llX" : "%llx"),
                              value);
  if (flags_ & oct)
    return __builtin_snprintf(
        buf, size, (flags_ & showbase) && value ? "0%llo" : "%llo", value);
  int n = __builtin_snprintf(buf, size, "%llu", value);
  if ((flags_ & showpos) && n > 0 && static_cast<size_t>(n + 1) < size) {
    __builtin_memmove(buf + 1, buf, static_cast<size_t>(n) + 1);
    buf[0] = '+';
    ++n;
  }
  return n;
}

template <typename Float>
int format_float(const ios_base& stream, char* buf, size_t size, Float value,
                 bool extended) {
  char fmt[16];
  const auto field = stream.flags() & ios_base::floatfield;
  const bool upper = stream.flags() & ios_base::uppercase;
  const char* spec = upper ? (extended ? "LG" : "G") : (extended ? "Lg" : "g");
  if (field == ios_base::floatfield)
    spec = upper ? (extended ? "LA" : "A") : (extended ? "La" : "a");
  else if (field == ios_base::fixed)
    spec = upper ? (extended ? "LF" : "F") : (extended ? "Lf" : "f");
  else if (field == ios_base::scientific)
    spec = upper ? (extended ? "LE" : "E") : (extended ? "Le" : "e");
  const char* alternate = stream.flags() & ios_base::showpoint ? "#" : "";
  const char* sign = stream.flags() & ios_base::showpos ? "+" : "";
  if (field == ios_base::floatfield)
    __builtin_snprintf(fmt, sizeof(fmt), "%%%s%s%s", alternate, sign, spec);
  else
    __builtin_snprintf(fmt, sizeof(fmt), "%%%s%s.%lld%s", alternate, sign,
                       static_cast<long long>(stream.precision()), spec);
  return __builtin_snprintf(buf, size, fmt, value);
}

int ios_base::_format_float(char* buf, size_t size, double value) const {
  return format_float(*this, buf, size, value, false);
}

int ios_base::_format_long_double(char* buf, size_t size,
                                  long double value) const {
  return format_float(*this, buf, size, value, true);
}

template class basic_ostream<char, char_traits<char>>;

} // namespace std
