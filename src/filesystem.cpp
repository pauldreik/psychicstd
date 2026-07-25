#include <cerrno>
#include <filesystem>
#include <sys/stat.h>

namespace std::filesystem {

bool exists(const path& value) noexcept {
  struct stat status;
  return ::stat(value.c_str(), &status) == 0;
}

bool is_directory(const path& value) noexcept {
  struct stat status;
  return ::stat(value.c_str(), &status) == 0 && S_ISDIR(status.st_mode);
}

bool is_regular_file(const path& value) noexcept {
  struct stat status;
  return ::stat(value.c_str(), &status) == 0 && S_ISREG(status.st_mode);
}

uintmax_t file_size(const path& value) {
  struct stat status;
  if (::stat(value.c_str(), &status) != 0)
    _PSYCHICSTD_THROW(filesystem_error("file_size", value,
                                       error_code(errno, generic_category())));
  return static_cast<uintmax_t>(status.st_size);
}

} // namespace std::filesystem
