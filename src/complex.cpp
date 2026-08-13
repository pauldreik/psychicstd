#include <complex>
#include <sstream>

namespace std::__complex_detail {

template <typename CharT, typename Traits, typename T>
basic_ostream<CharT, Traits>& write_complex(basic_ostream<CharT, Traits>& out,
                                            T real, T imag) {
  basic_ostringstream<CharT, Traits> field;
  field.flags(out.flags());
  field.imbue(out.getloc());
  field.precision(out.precision());
  field << '(' << real << ',' << imag << ')';
  return out << field.str();
}

#define _PSYCHICSTD_INSTANTIATE_COMPLEX_OUTPUT(CharT, T)                       \
  template basic_ostream<CharT, char_traits<CharT>>& write_complex(            \
      basic_ostream<CharT, char_traits<CharT>>&, T, T)

_PSYCHICSTD_INSTANTIATE_COMPLEX_OUTPUT(char, float);
_PSYCHICSTD_INSTANTIATE_COMPLEX_OUTPUT(char, double);
_PSYCHICSTD_INSTANTIATE_COMPLEX_OUTPUT(char, long double);
_PSYCHICSTD_INSTANTIATE_COMPLEX_OUTPUT(char, short);
_PSYCHICSTD_INSTANTIATE_COMPLEX_OUTPUT(char, unsigned short);
_PSYCHICSTD_INSTANTIATE_COMPLEX_OUTPUT(char, int);
_PSYCHICSTD_INSTANTIATE_COMPLEX_OUTPUT(char, unsigned int);
_PSYCHICSTD_INSTANTIATE_COMPLEX_OUTPUT(char, long);
_PSYCHICSTD_INSTANTIATE_COMPLEX_OUTPUT(char, unsigned long);
_PSYCHICSTD_INSTANTIATE_COMPLEX_OUTPUT(wchar_t, float);
_PSYCHICSTD_INSTANTIATE_COMPLEX_OUTPUT(wchar_t, double);
_PSYCHICSTD_INSTANTIATE_COMPLEX_OUTPUT(wchar_t, long double);
_PSYCHICSTD_INSTANTIATE_COMPLEX_OUTPUT(wchar_t, short);
_PSYCHICSTD_INSTANTIATE_COMPLEX_OUTPUT(wchar_t, unsigned short);
_PSYCHICSTD_INSTANTIATE_COMPLEX_OUTPUT(wchar_t, int);
_PSYCHICSTD_INSTANTIATE_COMPLEX_OUTPUT(wchar_t, unsigned int);
_PSYCHICSTD_INSTANTIATE_COMPLEX_OUTPUT(wchar_t, long);
_PSYCHICSTD_INSTANTIATE_COMPLEX_OUTPUT(wchar_t, unsigned long);

#undef _PSYCHICSTD_INSTANTIATE_COMPLEX_OUTPUT

}
