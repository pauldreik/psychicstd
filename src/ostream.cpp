#include <ostream>

namespace std {

ios_base& boolalpha(ios_base& stream) {
  stream.setf(ios_base::boolalpha);
  return stream;
}
ios_base& noboolalpha(ios_base& stream) {
  stream.unsetf(ios_base::boolalpha);
  return stream;
}
ios_base& showbase(ios_base& stream) {
  stream.setf(ios_base::showbase);
  return stream;
}
ios_base& noshowbase(ios_base& stream) {
  stream.unsetf(ios_base::showbase);
  return stream;
}
ios_base& showpos(ios_base& stream) {
  stream.setf(ios_base::showpos);
  return stream;
}
ios_base& noshowpos(ios_base& stream) {
  stream.unsetf(ios_base::showpos);
  return stream;
}
ios_base& showpoint(ios_base& stream) {
  stream.setf(ios_base::showpoint);
  return stream;
}
ios_base& noshowpoint(ios_base& stream) {
  stream.unsetf(ios_base::showpoint);
  return stream;
}
ios_base& uppercase(ios_base& stream) {
  stream.setf(ios_base::uppercase);
  return stream;
}
ios_base& nouppercase(ios_base& stream) {
  stream.unsetf(ios_base::uppercase);
  return stream;
}
ios_base& skipws(ios_base& stream) {
  stream.setf(ios_base::skipws);
  return stream;
}
ios_base& noskipws(ios_base& stream) {
  stream.unsetf(ios_base::skipws);
  return stream;
}
ios_base& left(ios_base& stream) {
  stream.setf(ios_base::left, ios_base::adjustfield);
  return stream;
}
ios_base& right(ios_base& stream) {
  stream.setf(ios_base::right, ios_base::adjustfield);
  return stream;
}
ios_base& internal(ios_base& stream) {
  stream.setf(ios_base::internal, ios_base::adjustfield);
  return stream;
}
ios_base& dec(ios_base& stream) {
  stream.setf(ios_base::dec, ios_base::basefield);
  return stream;
}
ios_base& hex(ios_base& stream) {
  stream.setf(ios_base::hex, ios_base::basefield);
  return stream;
}
ios_base& oct(ios_base& stream) {
  stream.setf(ios_base::oct, ios_base::basefield);
  return stream;
}
ios_base& fixed(ios_base& stream) {
  stream.setf(ios_base::fixed, ios_base::floatfield);
  return stream;
}
ios_base& scientific(ios_base& stream) {
  stream.setf(ios_base::scientific, ios_base::floatfield);
  return stream;
}
ios_base& defaultfloat(ios_base& stream) {
  stream.unsetf(ios_base::floatfield);
  return stream;
}
ios_base& hexfloat(ios_base& stream) {
  stream.setf(ios_base::fixed | ios_base::scientific, ios_base::floatfield);
  return stream;
}

int ios_base::_localize_number(char* out, size_t out_size, const char* input,
                               int input_size, char decimal_point,
                               char thousands_sep, const char* grouping,
                               size_t grouping_size) {
  int output_size = 0;
  auto emit = [&](char value) {
    if (static_cast<size_t>(output_size) < out_size)
      out[output_size++] = value;
  };
  int input_pos = 0;
  if (input_pos < input_size &&
      (input[input_pos] == '-' || input[input_pos] == '+'))
    emit(input[input_pos++]);
  int digits_start = input_pos;
  while (input_pos < input_size && input[input_pos] >= '0' &&
         input[input_pos] <= '9')
    ++input_pos;
  if (!grouping_size || !thousands_sep) {
    for (int i = digits_start; i < input_pos; ++i)
      emit(input[i]);
  } else {
    char reversed[40];
    int reversed_size = 0;
    int group_count = 0;
    size_t group_index = 0;
    int group_size = static_cast<unsigned char>(grouping[0]);
    for (int i = input_pos - 1; i >= digits_start; --i) {
      if (group_size > 0 && group_count == group_size) {
        if (reversed_size < static_cast<int>(sizeof(reversed)))
          reversed[reversed_size++] = thousands_sep;
        group_count = 0;
        if (group_index + 1 < grouping_size)
          group_size = static_cast<unsigned char>(grouping[++group_index]);
      }
      if (reversed_size < static_cast<int>(sizeof(reversed)))
        reversed[reversed_size++] = input[i];
      ++group_count;
    }
    for (int i = reversed_size - 1; i >= 0; --i)
      emit(reversed[i]);
  }
  for (; input_pos < input_size; ++input_pos)
    emit(input[input_pos] == '.' ? decimal_point : input[input_pos]);
  return output_size;
}

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
