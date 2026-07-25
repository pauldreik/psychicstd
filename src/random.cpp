#include <cmath>
#include <random>

#ifdef __APPLE__
extern "C" int open(const char*, int, ...) __asm("_open");
extern "C" long read(int, void*, unsigned long) __asm("_read");
extern "C" int close(int) __asm("_close");
#define _PSYCHICSTD_O_RDONLY 0x0000
#define _PSYCHICSTD_O_CLOEXEC 0x01000000
#else
#include <fcntl.h>
#include <unistd.h>
#define _PSYCHICSTD_O_RDONLY O_RDONLY
#define _PSYCHICSTD_O_CLOEXEC O_CLOEXEC
#endif

namespace std {

double __random_detail::normal_transform(double first, double second) {
  return ::sqrt(-2.0 * ::log(first)) *
         ::cos(2.0 * 3.14159265358979323846 * second);
}

random_device::random_device(const char*) {}

random_device::result_type random_device::operator()() {
  result_type value;
  int fd = ::open("/dev/urandom", _PSYCHICSTD_O_RDONLY | _PSYCHICSTD_O_CLOEXEC);
  if (fd >= 0) {
    long count = ::read(fd, &value, sizeof(value));
    ::close(fd);
    if (count == sizeof(value))
      return value;
  }

  static unsigned long long state =
      reinterpret_cast<unsigned long long>(&state);
  unsigned long long old = __atomic_load_n(&state, __ATOMIC_RELAXED);
  unsigned long long next;
  do {
    next = old * 6364136223846793005ULL + 1442695040888963407ULL;
  } while (!__atomic_compare_exchange_n(&state, &old, next, true,
                                        __ATOMIC_RELAXED, __ATOMIC_RELAXED));
  return static_cast<result_type>(next >> 33);
}

double random_device::entropy() const noexcept { return 32.0; }

mt19937::mt19937(result_type value) { seed(value); }

void mt19937::seed(result_type value) {
  mt_[0] = value;
  for (int i = 1; i < N; ++i)
    mt_[i] = 1812433253U * (mt_[i - 1] ^ (mt_[i - 1] >> 30)) + i;
  index_ = N;
}

void mt19937::generate() {
  static constexpr uint32_t mag[2] = {0, MATRIX_A};
  for (int i = 0; i < N - M; ++i) {
    uint32_t value = (mt_[i] & UPPER_MASK) | (mt_[i + 1] & LOWER_MASK);
    mt_[i] = mt_[i + M] ^ (value >> 1) ^ mag[value & 1];
  }
  for (int i = N - M; i < N - 1; ++i) {
    uint32_t value = (mt_[i] & UPPER_MASK) | (mt_[i + 1] & LOWER_MASK);
    mt_[i] = mt_[i + (M - N)] ^ (value >> 1) ^ mag[value & 1];
  }
  uint32_t value = (mt_[N - 1] & UPPER_MASK) | (mt_[0] & LOWER_MASK);
  mt_[N - 1] = mt_[M - 1] ^ (value >> 1) ^ mag[value & 1];
  index_ = 0;
}

mt19937::result_type mt19937::operator()() {
  if (index_ >= N)
    generate();
  uint32_t value = mt_[index_++];
  value ^= value >> 11;
  value ^= (value << 7) & 0x9d2c5680U;
  value ^= (value << 15) & 0xefc60000U;
  value ^= value >> 18;
  return value;
}

void mt19937::discard(unsigned long long count) {
  while (count--)
    operator()();
}

bool operator==(const mt19937& lhs, const mt19937& rhs) {
  if (lhs.index_ != rhs.index_)
    return false;
  for (int i = 0; i < mt19937::N; ++i)
    if (lhs.mt_[i] != rhs.mt_[i])
      return false;
  return true;
}

mt19937_64::mt19937_64(result_type value) { seed(value); }

void mt19937_64::seed(result_type value) {
  mt_[0] = value;
  for (int i = 1; i < N; ++i)
    mt_[i] = 6364136223846793005ULL * (mt_[i - 1] ^ (mt_[i - 1] >> 62)) + i;
  index_ = N;
}

void mt19937_64::generate() {
  static constexpr uint64_t mag[2] = {0, MATRIX_A};
  for (int i = 0; i < N - M; ++i) {
    uint64_t value = (mt_[i] & UPPER_MASK) | (mt_[i + 1] & LOWER_MASK);
    mt_[i] = mt_[i + M] ^ (value >> 1) ^ mag[value & 1];
  }
  for (int i = N - M; i < N - 1; ++i) {
    uint64_t value = (mt_[i] & UPPER_MASK) | (mt_[i + 1] & LOWER_MASK);
    mt_[i] = mt_[i + (M - N)] ^ (value >> 1) ^ mag[value & 1];
  }
  uint64_t value = (mt_[N - 1] & UPPER_MASK) | (mt_[0] & LOWER_MASK);
  mt_[N - 1] = mt_[M - 1] ^ (value >> 1) ^ mag[value & 1];
  index_ = 0;
}

mt19937_64::result_type mt19937_64::operator()() {
  if (index_ >= N)
    generate();
  uint64_t value = mt_[index_++];
  value ^= (value >> 29) & 0x5555555555555555ULL;
  value ^= (value << 17) & 0x71d67fffeda60000ULL;
  value ^= (value << 37) & 0xfff7eee000000000ULL;
  value ^= value >> 43;
  return value;
}

void mt19937_64::discard(unsigned long long count) {
  while (count--)
    operator()();
}

bool operator==(const mt19937_64& lhs, const mt19937_64& rhs) {
  if (lhs.index_ != rhs.index_)
    return false;
  for (int i = 0; i < mt19937_64::N; ++i)
    if (lhs.mt_[i] != rhs.mt_[i])
      return false;
  return true;
}

} // namespace std

#undef _PSYCHICSTD_O_RDONLY
#undef _PSYCHICSTD_O_CLOEXEC
