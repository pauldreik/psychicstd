#include <cerrno>
#include <dirent.h>
#include <filesystem>
#include <sys/stat.h>
#include <vector>

namespace std::filesystem {

path path::lexically_relative(const path& base) const {
  if (path_ == base.path_)
    return ".";
  if (path_.size() <= base.path_.size() ||
      path_.compare(0, base.path_.size(), base.path_) != 0 ||
      path_[base.path_.size()] != '/')
    return {};
  auto pos = base.path_.size();
  while (pos < path_.size() && path_[pos] == '/')
    ++pos;
  return path_.substr(pos);
}

path& path::replace_extension(const path& replacement) {
  auto slash = path_.find_last_of('/');
  auto dot = path_.find_last_of('.');
  if (dot != string::npos && dot != (slash == string::npos ? 0 : slash + 1) &&
      (slash == string::npos || dot > slash))
    path_.resize(dot);
  if (!replacement.empty()) {
    if (replacement.path_[0] != '.')
      path_ += '.';
    path_ += replacement.path_;
  }
  return *this;
}

struct recursive_directory_iterator::_state {
  unsigned references = 1;
  vector<directory_entry> entries;
  size_t position = 0;
};

static bool has_option(directory_options options,
                       directory_options option) noexcept {
  return (static_cast<unsigned char>(options) &
          static_cast<unsigned char>(option)) != 0;
}

static void add_directory_entries(const path& directory,
                                  directory_options options,
                                  vector<directory_entry>& entries) {
  DIR* handle = ::opendir(directory.c_str());
  if (!handle) {
    if (has_option(options, directory_options::skip_permission_denied))
      return;
    _PSYCHICSTD_THROW(filesystem_error("recursive_directory_iterator",
                                       directory,
                                       error_code(errno, generic_category())));
  }

  while (dirent* item = ::readdir(handle)) {
    string_view name(item->d_name);
    if (name == "." || name == "..")
      continue;

    path child = directory / path(name);
    entries.emplace_back(child);

    struct stat status;
    if (::lstat(child.c_str(), &status) != 0)
      continue;
    bool symlink = S_ISLNK(status.st_mode);
    if (symlink &&
        !has_option(options, directory_options::follow_directory_symlink))
      continue;
    if (symlink && ::stat(child.c_str(), &status) != 0)
      continue;
    if (S_ISDIR(status.st_mode))
      add_directory_entries(child, options, entries);
  }
  ::closedir(handle);
}

recursive_directory_iterator::recursive_directory_iterator(
    const path& directory, directory_options options) {
  state_ = new _state;
  add_directory_entries(directory, options, state_->entries);
  if (state_->entries.empty())
    release();
}

recursive_directory_iterator::recursive_directory_iterator(
    const recursive_directory_iterator& other) noexcept
    : state_(other.state_) {
  if (state_)
    ++state_->references;
}

recursive_directory_iterator::recursive_directory_iterator(
    recursive_directory_iterator&& other) noexcept
    : state_(other.state_) {
  other.state_ = nullptr;
}

recursive_directory_iterator& recursive_directory_iterator::operator=(
    const recursive_directory_iterator& other) noexcept {
  if (this == &other)
    return *this;
  release();
  state_ = other.state_;
  if (state_)
    ++state_->references;
  return *this;
}

recursive_directory_iterator& recursive_directory_iterator::operator=(
    recursive_directory_iterator&& other) noexcept {
  if (this == &other)
    return *this;
  release();
  state_ = other.state_;
  other.state_ = nullptr;
  return *this;
}

recursive_directory_iterator::~recursive_directory_iterator() { release(); }

const directory_entry&
recursive_directory_iterator::operator*() const noexcept {
  return state_->entries[state_->position];
}

const directory_entry*
recursive_directory_iterator::operator->() const noexcept {
  return &state_->entries[state_->position];
}

recursive_directory_iterator& recursive_directory_iterator::operator++() {
  if (++state_->position == state_->entries.size())
    release();
  return *this;
}

void recursive_directory_iterator::release() noexcept {
  if (state_ && --state_->references == 0)
    delete state_;
  state_ = nullptr;
}

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
