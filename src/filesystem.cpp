#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <filesystem>
#include <istream>
#include <limits.h>
#include <ostream>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <vector>

namespace std::filesystem {

namespace {

void clear_error(error_code& ec) noexcept { ec = error_code(); }

void set_error(error_code& ec, int value = errno) noexcept {
  ec = error_code(value, generic_category());
}

bool missing_path_error(int value) noexcept {
  return value == ENOENT || value == ENOTDIR;
}

file_type file_type_from_mode(mode_t mode) noexcept {
  if (S_ISREG(mode))
    return file_type::regular;
  if (S_ISDIR(mode))
    return file_type::directory;
  if (S_ISLNK(mode))
    return file_type::symlink;
  if (S_ISBLK(mode))
    return file_type::block;
  if (S_ISCHR(mode))
    return file_type::character;
  if (S_ISFIFO(mode))
    return file_type::fifo;
  if (S_ISSOCK(mode))
    return file_type::socket;
  return file_type::unknown;
}

bool same_file(FILE* source, const path& destination, error_code& ec) {
  struct stat source_status;
  if (::fstat(::fileno(source), &source_status) != 0) {
    set_error(ec);
    return false;
  }

  struct stat destination_status;
  if (::stat(destination.c_str(), &destination_status) != 0) {
    if (missing_path_error(errno))
      clear_error(ec);
    else
      set_error(ec);
    return false;
  }

  clear_error(ec);
  return source_status.st_dev == destination_status.st_dev &&
         source_status.st_ino == destination_status.st_ino;
}

class directory_handle {
public:
  explicit directory_handle(DIR* handle) noexcept : handle_(handle) {}
  ~directory_handle() {
    if (handle_)
      ::closedir(handle_);
  }

  DIR* get() const noexcept { return handle_; }
  explicit operator bool() const noexcept { return handle_; }

  int close() noexcept {
    DIR* handle = handle_;
    handle_ = nullptr;
    return ::closedir(handle);
  }

private:
  DIR* handle_;
};

void append_utf8(string& output, char32_t value) {
  if (value <= 0x7f) {
    output.push_back(static_cast<char>(value));
  } else if (value <= 0x7ff) {
    output.push_back(static_cast<char>(0xc0 | (value >> 6)));
    output.push_back(static_cast<char>(0x80 | (value & 0x3f)));
  } else if (value <= 0xffff) {
    output.push_back(static_cast<char>(0xe0 | (value >> 12)));
    output.push_back(static_cast<char>(0x80 | ((value >> 6) & 0x3f)));
    output.push_back(static_cast<char>(0x80 | (value & 0x3f)));
  } else {
    output.push_back(static_cast<char>(0xf0 | (value >> 18)));
    output.push_back(static_cast<char>(0x80 | ((value >> 12) & 0x3f)));
    output.push_back(static_cast<char>(0x80 | ((value >> 6) & 0x3f)));
    output.push_back(static_cast<char>(0x80 | (value & 0x3f)));
  }
}

template <typename Output> Output decode_utf8(string_view input) {
  Output result;
  for (size_t i = 0; i < input.size();) {
    unsigned char first = static_cast<unsigned char>(input[i++]);
    char32_t value = first;
    unsigned continuation = 0;
    if ((first & 0xe0) == 0xc0) {
      value = first & 0x1f;
      continuation = 1;
    } else if ((first & 0xf0) == 0xe0) {
      value = first & 0x0f;
      continuation = 2;
    } else if ((first & 0xf8) == 0xf0) {
      value = first & 0x07;
      continuation = 3;
    }
    while (continuation-- && i < input.size())
      value = (value << 6) | (static_cast<unsigned char>(input[i++]) & 0x3f);
    result.push_back(static_cast<typename Output::value_type>(value));
  }
  return result;
}

string_view next_path_component(string_view value, size_t& offset) noexcept {
  if (offset == value.size()) {
    offset = string_view::npos;
    return {};
  }
  if (offset == 0 && value[0] == '/') {
    while (offset < value.size() && value[offset] == '/')
      ++offset;
    if (offset == value.size())
      offset = string_view::npos;
    return "/";
  }

  size_t end = value.find('/', offset);
  string_view component = value.substr(offset, end - offset);
  if (end == string_view::npos) {
    offset = string_view::npos;
  } else {
    while (end < value.size() && value[end] == '/')
      ++end;
    offset = end;
  }
  return component;
}

template <typename Char, typename Traits>
basic_ostream<Char, Traits>& insert_path(basic_ostream<Char, Traits>& output,
                                         basic_string<Char, Traits> value) {
  basic_string<Char, Traits> quoted;
  quoted.reserve(value.size() + 2);
  quoted.push_back(Char('"'));
  for (Char character : value) {
    if (character == Char('"') || character == Char('\\'))
      quoted.push_back(Char('\\'));
    quoted.push_back(character);
  }
  quoted.push_back(Char('"'));
  return output << quoted;
}

template <typename Char, typename Traits>
basic_istream<Char, Traits>&
extract_path_string(basic_istream<Char, Traits>& input,
                    basic_string<Char, Traits>& value) {
  input >> ws;
  if (Traits::eq_int_type(input.peek(), Traits::eof()))
    return input;
  if (Traits::to_char_type(input.peek()) != Char('"'))
    return input >> value;

  (void)input.get();
  value.clear();
  bool escaped = false;
  Char character;
  while (input.get(character)) {
    if (!escaped && character == Char('"'))
      return input;
    if (!escaped && character == Char('\\')) {
      escaped = true;
    } else {
      value.push_back(character);
      escaped = false;
    }
  }
  input.setstate(ios_base::failbit);
  return input;
}

}

string path::from_wstring(wstring_view value) {
  std::string result;
  for (wchar_t character : value)
    append_utf8(result, static_cast<char32_t>(character));
  return result;
}

string path::from_u32string(u32string_view value) {
  std::string result;
  for (char32_t character : value)
    append_utf8(result, character);
  return result;
}

auto path::wstring() const -> std::wstring {
  return decode_utf8<std::wstring>(path_);
}

auto path::u32string() const -> std::u32string {
  return decode_utf8<std::u32string>(path_);
}

ostream& operator<<(ostream& output, const path& value) {
  return insert_path(output, value.string());
}

istream& operator>>(istream& input, path& value) {
  string decoded;
  extract_path_string(input, decoded);
  if (input)
    value.assign(static_cast<string&&>(decoded));
  return input;
}

wostream& operator<<(wostream& output, const path& value) {
  return insert_path(output, value.wstring());
}

wistream& operator>>(wistream& input, path& value) {
  wstring decoded;
  extract_path_string(input, decoded);
  if (input)
    value = path(decoded);
  return input;
}

path::iterator path::begin() const {
  if (path_.empty())
    return end();
  return iterator(this, 0);
}

path::iterator path::end() const { return iterator(this, string::npos); }

path::iterator::iterator(const path* owner, size_type offset) : owner_(owner) {
  set_component(offset);
}

void path::iterator::set_component(size_type offset) {
  offset_ = offset;
  if (offset == string_type::npos) {
    component_ = path();
    return;
  }

  const string_type& value = owner_->path_;
  if (offset == value.size()) {
    component_ = path();
  } else if (offset == 0 && value[0] == '/') {
    component_ = "/";
  } else {
    size_type component_end = value.find('/', offset);
    component_ = value.substr(offset, component_end - offset);
  }
}

path::iterator& path::iterator::operator++() {
  const string_type& value = owner_->path_;
  if (offset_ == value.size()) {
    set_component(string_type::npos);
    return *this;
  }

  size_type next;
  if (offset_ == 0 && value[0] == '/') {
    next = 1;
  } else {
    next = value.find('/', offset_);
    if (next == string_type::npos) {
      set_component(string_type::npos);
      return *this;
    }
  }

  while (next < value.size() && value[next] == '/')
    ++next;
  if (next == value.size() && offset_ == 0 && value[0] == '/')
    set_component(string_type::npos);
  else
    set_component(next);
  return *this;
}

path::iterator& path::iterator::operator--() {
  const string_type& value = owner_->path_;
  if (offset_ == string_type::npos) {
    if (value.back() == '/') {
      size_type last = value.size();
      while (last && value[last - 1] == '/')
        --last;
      set_component(last ? value.size() : 0);
    } else {
      size_type start = value.size();
      while (start && value[start - 1] != '/')
        --start;
      set_component(start);
    }
    return *this;
  }

  size_type previous_end = offset_;
  while (previous_end && value[previous_end - 1] == '/')
    --previous_end;
  if (!previous_end) {
    set_component(0);
    return *this;
  }
  size_type previous = previous_end;
  while (previous && value[previous - 1] != '/')
    --previous;
  set_component(previous);
  return *this;
}

path path::lexically_normal() const {
  if (path_.empty())
    return {};

  const bool absolute = path_[0] == '/';
  bool trailing_separator = path_.back() == '/';
  vector<std::string> parts;
  for (size_t begin = 0; begin < path_.size();) {
    while (begin < path_.size() && path_[begin] == '/')
      ++begin;
    auto end = path_.find('/', begin);
    if (end == std::string::npos)
      end = path_.size();
    std::string part = path_.substr(begin, end - begin);
    if (!part.empty() && part != ".") {
      if (part == ".." && !parts.empty() && parts.back() != "..") {
        parts.pop_back();
        if (end == path_.size())
          trailing_separator = true;
      } else if (part != ".." || !absolute) {
        parts.push_back(static_cast<std::string&&>(part));
      }
    } else if (part == "." && end == path_.size()) {
      trailing_separator = true;
    }
    begin = end + 1;
  }

  std::string result = absolute ? "/" : "";
  for (const auto& part : parts) {
    if (!result.empty() && result.back() != '/')
      result += '/';
    result += part;
  }
  if (result.empty())
    result = ".";
  if (trailing_separator && result != "/" && result != ".")
    result += '/';
  return result;
}

path path::lexically_relative(const path& base) const {
  if (is_absolute() != base.is_absolute())
    return {};

  vector<std::string> destination;
  vector<std::string> origin;
  for (const path& component : *this) {
    if (component.empty())
      destination.emplace_back();
    else if (component != "/" && component != ".")
      destination.push_back(component.string());
  }
  for (const path& component : base) {
    if (!component.empty() && component != "/" && component != ".")
      origin.push_back(component.string());
  }

  size_t common = 0;
  while (common < destination.size() && common < origin.size() &&
         destination[common] == origin[common])
    ++common;

  ptrdiff_t parents = 0;
  for (size_t i = common; i < origin.size(); ++i)
    parents += origin[i] == ".." ? -1 : 1;
  if (parents < 0)
    return {};

  std::string result;
  auto append = [&result](string_view component) {
    if (!result.empty())
      result += '/';
    result += component;
  };
  while (parents--)
    append("..");
  for (size_t i = common; i < destination.size(); ++i)
    append(destination[i]);
  return result.empty() ? path(".") : path(static_cast<std::string&&>(result));
}

path path::lexically_proximate(const path& base) const {
  path relative = lexically_relative(base);
  return relative.empty() ? *this : relative;
}

path path::relative_path() const {
  if (!is_absolute())
    return *this;
  size_t start = 0;
  while (start < path_.size() && path_[start] == '/')
    ++start;
  return path_.substr(start);
}

path path::parent_path() const {
  if (path_.empty())
    return {};
  size_t end = path_.size();
  while (end > 1 && path_[end - 1] == '/')
    --end;
  if (end != path_.size())
    return path_.substr(0, end);
  if (end == 1 && path_[0] == '/')
    return "/";
  size_t separator = path_.rfind('/', end - 1);
  if (separator == string::npos)
    return {};
  while (separator && path_[separator - 1] == '/')
    --separator;
  if (separator == 0)
    return "/";
  return path_.substr(0, separator);
}

bool path::has_relative_path() const noexcept {
  size_t start = 0;
  while (start < path_.size() && path_[start] == '/')
    ++start;
  return start != path_.size();
}

bool path::has_stem() const { return !stem().empty(); }

bool path::has_extension() const { return !extension().empty(); }

int path::compare(const path& other) const noexcept {
  size_t left = path_.empty() ? string_view::npos : 0;
  size_t right = other.path_.empty() ? string_view::npos : 0;
  while (left != string_view::npos && right != string_view::npos) {
    string_view left_component = next_path_component(path_, left);
    string_view right_component = next_path_component(other.path_, right);
    int difference = left_component.compare(right_component);
    if (difference)
      return difference;
  }
  return left == string_view::npos ? (right == string_view::npos ? 0 : -1) : 1;
}

path& path::remove_filename() {
  if (path_.empty() || path_.back() == '/')
    return *this;
  size_t separator = path_.find_last_of('/');
  path_.resize(separator == string::npos ? 0 : separator + 1);
  return *this;
}

path& path::replace_filename(const path& replacement) {
  remove_filename();
  return *this /= replacement;
}

size_t hash_value(const path& value) noexcept {
  size_t result = static_cast<size_t>(1469598103934665603ull);
  size_t offset = value.empty() ? string_view::npos : 0;
  while (offset != string_view::npos) {
    for (unsigned char byte : next_path_component(value.native(), offset)) {
      result ^= byte;
      result *= static_cast<size_t>(1099511628211ull);
    }
    result ^= 0xff;
    result *= static_cast<size_t>(1099511628211ull);
  }
  return result;
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
  directory_handle handle(::opendir(directory.c_str()));
  if (!handle) {
    if (has_option(options, directory_options::skip_permission_denied))
      return;
    _PSYCHICSTD_THROW(filesystem_error("recursive_directory_iterator",
                                       directory,
                                       error_code(errno, generic_category())));
  }

  while (dirent* item = ::readdir(handle.get())) {
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
}

recursive_directory_iterator::recursive_directory_iterator(
    const path& directory, directory_options options) {
  state_ = new _state;
  _PSYCHICSTD_TRY {
    add_directory_entries(directory, options, state_->entries);
  }
  _PSYCHICSTD_CATCH(...) {
    release();
    _PSYCHICSTD_RETHROW;
  }
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

struct directory_iterator::_state {
  unsigned references = 1;
  vector<directory_entry> entries;
  size_t position = 0;
};

static bool read_directory(const path& directory,
                           vector<directory_entry>& entries, error_code& ec) {
  directory_handle handle(::opendir(directory.c_str()));
  if (!handle) {
    set_error(ec);
    return false;
  }

  errno = 0;
  while (dirent* item = ::readdir(handle.get())) {
    string_view name(item->d_name);
    if (name != "." && name != "..")
      entries.emplace_back(directory / path(name));
    errno = 0;
  }
  int read_error = errno;
  if (handle.close() != 0 && !read_error)
    read_error = errno;
  if (read_error) {
    set_error(ec, read_error);
    return false;
  }
  clear_error(ec);
  return true;
}

directory_iterator::directory_iterator(const path& directory) {
  error_code ec;
  state_ = new _state;
  _PSYCHICSTD_TRY {
    if (!read_directory(directory, state_->entries, ec)) {
      release();
      _PSYCHICSTD_THROW(filesystem_error("directory_iterator", directory, ec));
    }
  }
  _PSYCHICSTD_CATCH(...) {
    release();
    _PSYCHICSTD_RETHROW;
  }
  if (state_->entries.empty())
    release();
}

directory_iterator::directory_iterator(const path& directory, error_code& ec) {
  state_ = new _state;
  _PSYCHICSTD_TRY {
    if (!read_directory(directory, state_->entries, ec) ||
        state_->entries.empty())
      release();
  }
  _PSYCHICSTD_CATCH(...) {
    release();
    _PSYCHICSTD_RETHROW;
  }
}

directory_iterator::directory_iterator(const directory_iterator& other) noexcept
    : state_(other.state_) {
  if (state_)
    ++state_->references;
}

directory_iterator::directory_iterator(directory_iterator&& other) noexcept
    : state_(other.state_) {
  other.state_ = nullptr;
}

directory_iterator&
directory_iterator::operator=(const directory_iterator& other) noexcept {
  if (this == &other)
    return *this;
  release();
  state_ = other.state_;
  if (state_)
    ++state_->references;
  return *this;
}

directory_iterator&
directory_iterator::operator=(directory_iterator&& other) noexcept {
  if (this == &other)
    return *this;
  release();
  state_ = other.state_;
  other.state_ = nullptr;
  return *this;
}

directory_iterator::~directory_iterator() { release(); }

const directory_entry& directory_iterator::operator*() const noexcept {
  return state_->entries[state_->position];
}

const directory_entry* directory_iterator::operator->() const noexcept {
  return &state_->entries[state_->position];
}

directory_iterator& directory_iterator::operator++() {
  if (++state_->position == state_->entries.size())
    release();
  return *this;
}

directory_iterator& directory_iterator::increment(error_code& ec) {
  clear_error(ec);
  return operator++();
}

void directory_iterator::release() noexcept {
  if (state_ && --state_->references == 0)
    delete state_;
  state_ = nullptr;
}

bool exists(const path& value) noexcept {
  struct stat status;
  return ::stat(value.c_str(), &status) == 0;
}

bool exists(const path& value, error_code& ec) noexcept {
  struct stat status;
  if (::stat(value.c_str(), &status) == 0) {
    clear_error(ec);
    return true;
  }
  if (missing_path_error(errno))
    clear_error(ec);
  else
    set_error(ec);
  return false;
}

bool is_directory(const path& value) noexcept {
  struct stat status;
  return ::stat(value.c_str(), &status) == 0 && S_ISDIR(status.st_mode);
}

bool is_directory(const path& value, error_code& ec) noexcept {
  struct stat status;
  if (::stat(value.c_str(), &status) == 0) {
    clear_error(ec);
    return S_ISDIR(status.st_mode);
  }
  if (missing_path_error(errno))
    clear_error(ec);
  else
    set_error(ec);
  return false;
}

bool is_regular_file(const path& value) noexcept {
  struct stat status;
  return ::stat(value.c_str(), &status) == 0 && S_ISREG(status.st_mode);
}

bool is_regular_file(const path& value, error_code& ec) noexcept {
  struct stat status;
  if (::stat(value.c_str(), &status) == 0) {
    clear_error(ec);
    return S_ISREG(status.st_mode);
  }
  if (missing_path_error(errno))
    clear_error(ec);
  else
    set_error(ec);
  return false;
}

bool directory_entry::is_regular_file() const noexcept {
  return filesystem::is_regular_file(path_);
}

bool directory_entry::is_directory() const noexcept {
  return filesystem::is_directory(path_);
}

file_status directory_entry::status() const {
  return filesystem::status(path_);
}

file_status directory_entry::symlink_status() const {
  return filesystem::symlink_status(path_);
}

bool is_symlink(const path& value, error_code& ec) noexcept {
  struct stat status;
  if (::lstat(value.c_str(), &status) == 0) {
    clear_error(ec);
    return S_ISLNK(status.st_mode);
  }
  if (missing_path_error(errno))
    clear_error(ec);
  else
    set_error(ec);
  return false;
}

uintmax_t file_size(const path& value) {
  struct stat status;
  if (::stat(value.c_str(), &status) != 0)
    _PSYCHICSTD_THROW(filesystem_error("file_size", value,
                                       error_code(errno, generic_category())));
  return static_cast<uintmax_t>(status.st_size);
}

bool equivalent(const path& first, const path& second,
                error_code& ec) noexcept {
  struct stat first_status;
  if (::stat(first.c_str(), &first_status) != 0) {
    set_error(ec);
    return false;
  }
  struct stat second_status;
  if (::stat(second.c_str(), &second_status) != 0) {
    set_error(ec);
    return false;
  }
  clear_error(ec);
  return first_status.st_dev == second_status.st_dev &&
         first_status.st_ino == second_status.st_ino;
}

bool equivalent(const path& first, const path& second) {
  error_code ec;
  bool result = equivalent(first, second, ec);
  if (ec)
    _PSYCHICSTD_THROW(filesystem_error("equivalent", first, ec));
  return result;
}

file_status status(const path& value) {
  error_code ec;
  file_status result = status(value, ec);
  if (ec)
    _PSYCHICSTD_THROW(filesystem_error("status", value, ec));
  return result;
}

file_status status(const path& value, error_code& ec) noexcept {
  struct stat result;
  if (::stat(value.c_str(), &result) != 0) {
    if (missing_path_error(errno)) {
      clear_error(ec);
      return file_status(file_type::not_found);
    }
    set_error(ec);
    return file_status(file_type::none);
  }
  clear_error(ec);
  return file_status(file_type_from_mode(result.st_mode),
                     static_cast<perms>(result.st_mode & 07777));
}

file_status symlink_status(const path& value) {
  error_code ec;
  file_status result = symlink_status(value, ec);
  if (ec)
    _PSYCHICSTD_THROW(filesystem_error("symlink_status", value, ec));
  return result;
}

file_status symlink_status(const path& value, error_code& ec) noexcept {
  struct stat result;
  if (::lstat(value.c_str(), &result) != 0) {
    if (missing_path_error(errno)) {
      clear_error(ec);
      return file_status(file_type::not_found);
    }
    set_error(ec);
    return file_status(file_type::none);
  }
  clear_error(ec);
  return file_status(file_type_from_mode(result.st_mode),
                     static_cast<perms>(result.st_mode & 07777));
}

space_info space(const path& value) {
  struct statvfs result;
  if (::statvfs(value.c_str(), &result) != 0) {
    error_code ec;
    set_error(ec);
    _PSYCHICSTD_THROW(filesystem_error("space", value, ec));
  }
  return {
      static_cast<uintmax_t>(result.f_blocks) * result.f_frsize,
      static_cast<uintmax_t>(result.f_bfree) * result.f_frsize,
      static_cast<uintmax_t>(result.f_bavail) * result.f_frsize,
  };
}

uintmax_t file_size(const path& value, error_code& ec) noexcept {
  struct stat status;
  if (::stat(value.c_str(), &status) != 0) {
    set_error(ec);
    return static_cast<uintmax_t>(-1);
  }
  clear_error(ec);
  return static_cast<uintmax_t>(status.st_size);
}

path temp_directory_path(error_code& ec) {
  const char* value = ::getenv("TMPDIR");
  path result(value && *value ? value : "/tmp");
  struct stat status;
  if (::stat(result.c_str(), &status) != 0) {
    set_error(ec);
    return {};
  }
  if (!S_ISDIR(status.st_mode)) {
    set_error(ec, ENOTDIR);
    return {};
  }
  clear_error(ec);
  return result;
}

path temp_directory_path() {
  error_code ec;
  path result = temp_directory_path(ec);
  if (ec)
    _PSYCHICSTD_THROW(filesystem_error("temp_directory_path", ec));
  return result;
}

path current_path(error_code& ec) {
  char buffer[PATH_MAX];
  if (!::getcwd(buffer, sizeof(buffer))) {
    set_error(ec);
    return {};
  }
  clear_error(ec);
  return buffer;
}

path current_path() {
  error_code ec;
  path result = current_path(ec);
  if (ec)
    _PSYCHICSTD_THROW(filesystem_error("current_path", ec));
  return result;
}

void current_path(const path& value, error_code& ec) noexcept {
  if (::chdir(value.c_str()) != 0)
    set_error(ec);
  else
    clear_error(ec);
}

path absolute(const path& value, error_code& ec) {
  if (!value.empty() && value.native()[0] == '/') {
    clear_error(ec);
    return value;
  }
  path cwd = current_path(ec);
  return ec ? path() : cwd / value;
}

path absolute(const path& value) {
  error_code ec;
  path result = absolute(value, ec);
  if (ec)
    _PSYCHICSTD_THROW(filesystem_error("absolute", value, ec));
  return result;
}

path weakly_canonical(const path& value, error_code& ec) {
  char* resolved = ::realpath(value.c_str(), nullptr);
  if (resolved) {
    path result(resolved);
    ::free(resolved);
    clear_error(ec);
    return result;
  }
  if (missing_path_error(errno)) {
    // This fallback does not resolve symlinks in an existing path prefix.
    path result = absolute(value, ec);
    return ec ? path() : result.lexically_normal();
  }
  set_error(ec);
  return {};
}

path canonical(const path& value, error_code& ec) noexcept {
  // Unlike weakly_canonical, the path must actually exist: realpath()
  // failing (e.g. ENOENT) is a real error here, not something to fall back
  // on.
  char* resolved = ::realpath(value.c_str(), nullptr);
  if (!resolved) {
    set_error(ec);
    return {};
  }
  path result(resolved);
  ::free(resolved);
  clear_error(ec);
  return result;
}

path canonical(const path& value) {
  error_code ec;
  path result = canonical(value, ec);
  if (ec)
    _PSYCHICSTD_THROW(filesystem_error("canonical", value, ec));
  return result;
}

file_time_type last_write_time(const path& value, error_code& ec) noexcept {
  struct stat status;
  if (::stat(value.c_str(), &status) != 0) {
    set_error(ec);
    return file_time_type::min();
  }
  clear_error(ec);
#if defined(__APPLE__)
  const auto seconds = status.st_mtimespec.tv_sec;
  const auto nanoseconds = status.st_mtimespec.tv_nsec;
#else
  const auto seconds = status.st_mtim.tv_sec;
  const auto nanoseconds = status.st_mtim.tv_nsec;
#endif
  return file_time_type(
      chrono::nanoseconds(seconds * 1000000000LL + nanoseconds));
}

bool remove(const path& value, error_code& ec) noexcept {
  if (::remove(value.c_str()) == 0) {
    clear_error(ec);
    return true;
  }
  if (missing_path_error(errno)) {
    clear_error(ec);
    return false;
  }
  set_error(ec);
  return false;
}

bool remove(const path& value) {
  error_code ec;
  bool result = remove(value, ec);
  if (ec)
    _PSYCHICSTD_THROW(filesystem_error("remove", value, ec));
  return result;
}

namespace {

constexpr uintmax_t remove_all_error = static_cast<uintmax_t>(-1);

uintmax_t remove_all_impl(const path& value, error_code& ec) {
  struct stat status;
  if (::lstat(value.c_str(), &status) != 0) {
    if (missing_path_error(errno)) {
      clear_error(ec);
      return 0;
    }
    set_error(ec);
    return remove_all_error;
  }

  uintmax_t count = 0;
  if (S_ISDIR(status.st_mode)) {
    DIR* dir = ::opendir(value.c_str());
    if (!dir) {
      set_error(ec);
      return remove_all_error;
    }
    while (struct dirent* entry = ::readdir(dir)) {
      string_view name(entry->d_name);
      if (name == "." || name == "..")
        continue;
      uintmax_t sub = remove_all_impl(value / entry->d_name, ec);
      if (ec) {
        ::closedir(dir);
        return remove_all_error;
      }
      count += sub;
    }
    ::closedir(dir);
  }

  if (::remove(value.c_str()) != 0) {
    if (!missing_path_error(errno)) {
      set_error(ec);
      return remove_all_error;
    }
  } else {
    ++count;
  }
  clear_error(ec);
  return count;
}

}

uintmax_t remove_all(const path& value, error_code& ec) {
  return remove_all_impl(value, ec);
}

uintmax_t remove_all(const path& value) {
  error_code ec;
  uintmax_t result = remove_all(value, ec);
  if (ec)
    _PSYCHICSTD_THROW(filesystem_error("remove_all", value, ec));
  return result;
}

static bool copy_file_impl(const path& source, const path& destination,
                           bool overwrite, error_code& ec) {
  FILE* input = ::fopen(source.c_str(), "rb");
  if (!input) {
    set_error(ec);
    return false;
  }

  if (overwrite) {
    const bool source_is_destination = same_file(input, destination, ec);
    if (ec) {
      ::fclose(input);
      return false;
    }
    if (source_is_destination) {
      ::fclose(input);
      set_error(ec, EEXIST);
      return false;
    }
  }

  FILE* output = ::fopen(destination.c_str(), overwrite ? "wb" : "wbx");
  if (!output) {
    const int open_error = errno;
    ::fclose(input);
    set_error(ec, open_error);
    return false;
  }

  bool ok = true;
  char buffer[8192];
  while (size_t count = ::fread(buffer, 1, sizeof(buffer), input)) {
    if (::fwrite(buffer, 1, count, output) != count) {
      ok = false;
      break;
    }
  }
  if (::ferror(input))
    ok = false;
  const int io_error = errno;
  if (::fclose(output) != 0)
    ok = false;
  ::fclose(input);
  if (!ok) {
    set_error(ec, io_error ? io_error : EIO);
    return false;
  }
  clear_error(ec);
  return true;
}

bool copy_file(const path& source, const path& destination, error_code& ec) {
  return copy_file_impl(source, destination, false, ec);
}

bool copy_file(const path& source, const path& destination,
               copy_options options) {
  error_code ec;
  if (options == copy_options::skip_existing && exists(destination, ec)) {
    if (ec)
      _PSYCHICSTD_THROW(filesystem_error("copy_file", destination, ec));
    return false;
  }

  bool result = copy_file_impl(source, destination,
                               options == copy_options::overwrite_existing, ec);
  if (ec)
    _PSYCHICSTD_THROW(filesystem_error("copy_file", source, ec));
  return result;
}

bool create_directory(const path& value, error_code& ec) noexcept {
  if (::mkdir(value.c_str(), 0777) == 0) {
    clear_error(ec);
    return true;
  }
  if (errno == EEXIST && is_directory(value)) {
    clear_error(ec);
    return false;
  }
  set_error(ec);
  return false;
}

bool create_directories(const path& value, error_code& ec) {
  if (value.empty()) {
    clear_error(ec);
    return false;
  }
  if (is_directory(value, ec))
    return false;
  if (ec)
    return false;

  path parent = value.parent_path();
  if (!parent.empty() && !is_directory(parent, ec)) {
    if (ec || !create_directories(parent, ec))
      if (ec)
        return false;
  }
  return create_directory(value, ec);
}

bool create_directories(const path& value) {
  error_code ec;
  bool result = create_directories(value, ec);
  if (ec)
    _PSYCHICSTD_THROW(filesystem_error("create_directories", value, ec));
  return result;
}

void permissions(const path& value, perms permissions_value,
                 perm_options options, error_code& ec) noexcept {
  mode_t mode = static_cast<mode_t>(permissions_value) & 07777;
  if (options == perm_options::add || options == perm_options::remove) {
    struct stat status;
    if (::stat(value.c_str(), &status) != 0) {
      set_error(ec);
      return;
    }
    if (options == perm_options::add)
      mode |= status.st_mode & 07777;
    else
      mode = (status.st_mode & 07777) & ~mode;
  }
  if (::chmod(value.c_str(), mode) != 0)
    set_error(ec);
  else
    clear_error(ec);
}

void rename(const path& source, const path& destination,
            error_code& ec) noexcept {
  if (::rename(source.c_str(), destination.c_str()) != 0)
    set_error(ec);
  else
    clear_error(ec);
}

}
