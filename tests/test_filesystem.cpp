#include "psyassert.h"
#include <filesystem>
#include <fstream>
#include <queue>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <string_view>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

static std::vector<std::string> components(const std::filesystem::path& path) {
  std::vector<std::string> result;
  for (const auto& component : path)
    result.push_back(component.string());
  return result;
}

int main() {
  namespace fs = std::filesystem;
  const std::string unique =
      "psychicstd_test_filesystem_" +
      std::to_string(static_cast<unsigned long long>(::getpid()));

  fs::path p1("/tmp/some/dir/file.txt");
  static_assert(fs::path::preferred_separator == '/');
  psyassert(p1.filename() == fs::path("file.txt"));
  psyassert(p1.stem() == fs::path("file"));
  psyassert(p1.extension() == fs::path(".txt"));
  psyassert(p1.parent_path() == fs::path("/tmp/some/dir"));
  psyassert(p1.string() == "/tmp/some/dir/file.txt");
  std::string converted = p1;
  psyassert(converted == p1.string());
  std::ostringstream path_stream;
  path_stream << p1;
  psyassert(path_stream.str() == '"' + p1.string() + '"');
  std::ostringstream escaped_path_stream;
  escaped_path_stream << fs::path("a b\\c");
  psyassert(escaped_path_stream.str() == "\"a b\\\\c\"");
  fs::path extracted_path;
  std::istringstream escaped_path_input(escaped_path_stream.str());
  escaped_path_input >> extracted_path;
  psyassert(extracted_path == fs::path("a b\\c"));
  std::istringstream unquoted_path_input("unquoted/path");
  unquoted_path_input >> extracted_path;
  psyassert(extracted_path == fs::path("unquoted/path"));
  std::wostringstream wide_path_stream;
  wide_path_stream << fs::path("wide path");
  psyassert(wide_path_stream.str() == L"\"wide path\"");
  std::wistringstream wide_path_input(wide_path_stream.str());
  wide_path_input >> extracted_path;
  psyassert(extracted_path == fs::path("wide path"));

  fs::path p2 = std::string("noext");
  psyassert(p2.extension().empty());
  psyassert(p2.stem() == fs::path("noext"));
  psyassert(fs::path("..").stem() == fs::path(".."));
  psyassert(fs::path("..").extension().empty());

  fs::path joined = fs::path("/tmp/some") / "dir" / fs::path("file.txt");
  psyassert(joined == p1);
  psyassert((fs::path("a") / fs::path()).string() == "a/");
  psyassert(fs::path("a") / fs::path("/b") == fs::path("/b"));
  psyassert(p1.lexically_relative("/tmp/some") == fs::path("dir/file.txt"));
  psyassert(p1.lexically_relative(p1) == fs::path("."));
  psyassert(fs::path("a/b/").lexically_relative("a") == fs::path("b/"));
  psyassert(fs::path("archive.tar.gz").replace_extension() ==
            fs::path("archive.tar"));
  psyassert(fs::path("archive.tar.gz").replace_extension("txt") ==
            fs::path("archive.tar.txt"));

  std::u8string u8 = p1.u8string();
  fs::path from_u8{std::u8string_view(u8)};
  psyassert(from_u8 == p1);
  psyassert(fs::path(L"wide/path").wstring() == L"wide/path");
  psyassert(fs::path(U"unicode/path").u32string() == U"unicode/path");
  psyassert(fs::path("parent/child").has_parent_path());
  psyassert(fs::path("parent/child").has_filename());
  psyassert(!fs::path("parent/").has_filename());
  psyassert(fs::path("/tmp").is_absolute());
  psyassert(fs::path("tmp").is_relative());
  psyassert(fs::path("a//b/../c/./").lexically_normal() == fs::path("a/c/"));
  psyassert(fs::path("a/./b/..").lexically_normal() == fs::path("a/"));
  psyassert(fs::path("/a//d").lexically_relative("/a/b/c") ==
            fs::path("../../d"));
  psyassert(fs::path("a/b/c").lexically_relative("a/b/c/x/y") ==
            fs::path("../.."));
  psyassert(fs::path("a/b").lexically_relative("c/d") == fs::path("../../a/b"));
  psyassert(fs::path("/a").lexically_relative("/a/.") == fs::path("."));
  fs::path appended = "suffix";
  appended += ".txt";
  appended += fs::path::value_type('!');
  appended.append("child");
  psyassert(appended == fs::path("suffix.txt!/child"));
  appended = "suffix";
  appended += ".txt";
  psyassert(appended == fs::path("suffix.txt"));
  psyassert(fs::path("native").native() == "native");
  psyassert(fs::path("generic/path").generic_string() == "generic/path");
  psyassert(fs::path("/root/path").root_name().empty());
  psyassert(fs::path("/root/path").root_directory() == fs::path("/"));
  psyassert(fs::path("/root/path").root_path() == fs::path("/"));
  psyassert(fs::path("/root/path").relative_path() == fs::path("root/path"));
  psyassert(fs::path("relative/path").relative_path() ==
            fs::path("relative/path"));
  psyassert(fs::path("/root/path").has_root_path());
  psyassert(!fs::path("/root/path").has_root_name());
  psyassert(fs::path("/root/path").has_root_directory());
  psyassert(fs::path("/root/path").has_relative_path());
  psyassert(!fs::path("/").has_relative_path());
  psyassert(fs::path("archive.tar").has_stem());
  psyassert(fs::path("archive.tar").has_extension());
  psyassert(!fs::path("archive").has_extension());
  psyassert(fs::path("a//b").compare(fs::path("a/b")) == 0);
  psyassert(fs::path("a/b").compare(fs::path("a/c")) < 0);
  psyassert(fs::path("a/c").compare(fs::path("a/b")) > 0);
  psyassert(fs::path("a//b") == fs::path("a/b"));
  psyassert(fs::path("/root/child").lexically_proximate("/root") ==
            fs::path("child"));
  fs::path cleared("content");
  cleared.clear();
  psyassert(cleared.empty());
  fs::path without_filename("parent/file.txt");
  without_filename.remove_filename();
  psyassert(without_filename == fs::path("parent/"));
  without_filename.replace_filename("replacement.txt");
  psyassert(without_filename == fs::path("parent/replacement.txt"));
  psyassert(fs::path("single").remove_filename().empty());
  psyassert(fs::path("parent/").remove_filename() == fs::path("parent/"));
  psyassert(fs::path("/").parent_path() == fs::path("/"));
  psyassert(fs::path("/a").parent_path() == fs::path("/"));
  psyassert(fs::path("a//b").parent_path() == fs::path("a"));
  fs::path preferred("parent/child");
  psyassert(&preferred.make_preferred() == &preferred);
  psyassert(preferred == fs::path("parent/child"));
  fs::path assigned;
  assigned.assign(std::string("assigned"));
  assigned.concat(std::string("-tail"));
  psyassert(assigned == fs::path("assigned-tail"));
  fs::path swapped("left");
  fs::path swap_right("right");
  swapped.swap(swap_right);
  psyassert(swapped == fs::path("right") && swap_right == fs::path("left"));
  fs::swap(swapped, swap_right);
  psyassert(swapped == fs::path("left") && swap_right == fs::path("right"));
  psyassert(fs::hash_value(fs::path("same")) ==
            fs::hash_value(fs::path("same")));
  psyassert(fs::hash_value(fs::path("same//path")) ==
            fs::hash_value(fs::path("same/path")));

  psyassert(components(fs::path()).empty());
  psyassert(components(fs::path("/")) == std::vector<std::string>{"/"});
  psyassert(components(fs::path("alpha")) == std::vector<std::string>{"alpha"});
  psyassert(components(fs::path("alpha/beta")) ==
            (std::vector<std::string>{"alpha", "beta"}));
  psyassert(components(fs::path("/alpha//beta/")) ==
            (std::vector<std::string>{"/", "alpha", "beta", ""}));

  const fs::path reverse_path("/alpha/beta/");
  auto component = reverse_path.end();
  psyassert((--component)->empty());
  psyassert((--component)->string() == "beta");
  auto beta = component--;
  psyassert(beta->string() == "beta");
  psyassert(component->string() == "alpha");
  psyassert((--component)->string() == "/");

  std::queue<fs::path> q;
  q.push(std::string("/tmp/a"));
  q.push(std::string("/tmp/b"));
  psyassert(q.front().string() == "/tmp/a");

  const std::string tmp_name = unique + "_tmp.txt";
  {
    std::ofstream out(tmp_name);
    out << "hi";
  }
  fs::path existing(tmp_name);
  psyassert(fs::exists(existing));
  psyassert(fs::equivalent(existing, existing));
  psyassert(fs::is_regular_file(existing));
  psyassert(!fs::is_directory(existing));
  psyassert(fs::file_size(existing) == 2);
  psyassert(!fs::exists(fs::path("does_not_exist_hopefully.txt")));
  std::remove(tmp_name.c_str());

  const std::string tree = unique + "_tree";
  const std::string subtree = tree + "/sub";
  const std::string child = subtree + "/file.txt";
  ::mkdir(tree.c_str(), 0700);
  ::mkdir(subtree.c_str(), 0700);
  {
    std::ofstream out(child);
    out << "child";
  }
  bool saw_subtree = false;
  bool saw_child = false;
  for (const auto& entry : fs::recursive_directory_iterator(
           tree, fs::directory_options::follow_directory_symlink |
                     fs::directory_options::skip_permission_denied)) {
    auto relative = entry.path().lexically_relative(tree);
    saw_subtree |= relative == fs::path("sub");
    saw_child |= relative == fs::path("sub/file.txt");
  }
  psyassert(saw_subtree);
  psyassert(saw_child);
  std::remove(child.c_str());
  ::rmdir(subtree.c_str());
  ::rmdir(tree.c_str());

  std::error_code ec;
  fs::path cwd = fs::current_path(ec);
  psyassert(!ec && !cwd.empty());
  psyassert(fs::space(cwd).capacity >= fs::space(cwd).available);
  fs::current_path(cwd, ec);
  psyassert(!ec);

  const char* previous_tmpdir = ::getenv("TMPDIR");
  const bool had_tmpdir = previous_tmpdir;
  const std::string saved_tmpdir = previous_tmpdir ? previous_tmpdir : "";
  const std::string missing_tmpdir = unique + "_missing";
  psyassert(::setenv("TMPDIR", missing_tmpdir.c_str(), 1) == 0);
  psyassert(fs::temp_directory_path(ec).empty());
  psyassert(ec);
  if (had_tmpdir)
    psyassert(::setenv("TMPDIR", saved_tmpdir.c_str(), 1) == 0);
  else
    psyassert(::unsetenv("TMPDIR") == 0);
  ec = {};
  psyassert(fs::is_directory(fs::temp_directory_path(ec), ec));
  psyassert(!ec);

  fs::path ops_root = unique + "_ops";
  fs::path nested = ops_root / "a" / "b";
  psyassert(fs::create_directories(nested, ec));
  psyassert(!ec);
  psyassert(!fs::create_directories(nested, ec));
  psyassert(!ec);

  fs::path source_file = ops_root / "source.txt";
  {
    std::ofstream out(source_file);
    out << "copy me";
  }
  fs::path copied_file = ops_root / "copied.txt";
  psyassert(fs::copy_file(source_file, copied_file, ec));
  psyassert(!ec && fs::file_size(copied_file, ec) == 7);
  psyassert(!ec);
  psyassert(!fs::copy_file(source_file, copied_file, ec));
  psyassert(ec);
  ec = {};
  {
    std::ofstream out(copied_file);
    out << "old";
  }
  psyassert(fs::copy_file(source_file, copied_file,
                          fs::copy_options::overwrite_existing));
  psyassert(fs::file_size(copied_file) == 7);
  bool same_file_threw = false;
  try {
    (void)fs::copy_file(source_file, source_file,
                        fs::copy_options::overwrite_existing);
  } catch (const fs::filesystem_error&) {
    same_file_threw = true;
  }
  psyassert(same_file_threw && fs::file_size(source_file) == 7);
  fs::permissions(copied_file, fs::perms::owner_read | fs::perms::owner_write,
                  fs::perm_options::replace, ec);
  psyassert(!ec);
  psyassert((fs::status(copied_file).permissions() & fs::perms::owner_write) !=
            fs::perms::none);
  psyassert(!fs::copy_file(source_file, copied_file,
                           fs::copy_options::skip_existing));
  psyassert(((fs::perms::owner_read | fs::perms::owner_write) &
             fs::perms::owner_write) != fs::perms::none);
  auto write_time = fs::last_write_time(copied_file, ec);
  psyassert(!ec);
  (void)write_time;
  psyassert(fs::last_write_time(unique + "_missing", ec) ==
            fs::file_time_type::min());
  psyassert(ec);
  ec = {};
  psyassert(!fs::is_symlink(copied_file, ec));
  psyassert(!ec);

  bool saw_nested_parent = false;
  bool saw_source = false;
  for (fs::directory_iterator it(ops_root, ec), last; it != last;
       it.increment(ec)) {
    psyassert(!ec);
    auto filename = it->path().filename();
    saw_nested_parent |= filename == fs::path("a");
    saw_source |= filename == fs::path("source.txt");
  }
  psyassert(!ec && saw_nested_parent && saw_source);

  fs::directory_iterator postincrement(ops_root, ec);
  psyassert(!ec);
  const fs::path first_entry = postincrement->path();
  auto previous = postincrement++;
  psyassert((*previous).path() == first_entry);

  bool range_iterated = false;
  for (const auto& entry : fs::directory_iterator(ops_root)) {
    psyassert(!entry.path().empty());
    range_iterated = true;
  }
  psyassert(range_iterated);

  fs::path renamed_file = ops_root / "renamed.txt";
  fs::rename(copied_file, renamed_file, ec);
  psyassert(!ec && fs::exists(renamed_file, ec));
  psyassert(!ec);
  psyassert(!fs::absolute(renamed_file, ec).empty());
  psyassert(!ec);
  psyassert(!fs::absolute(renamed_file).empty());
  psyassert(!fs::weakly_canonical(renamed_file, ec).empty());
  psyassert(!ec);

  psyassert(fs::remove(source_file, ec));
  psyassert(fs::remove(renamed_file, ec));
  psyassert(fs::remove(nested, ec));
  psyassert(fs::remove(ops_root / "a", ec));
  psyassert(fs::remove(ops_root, ec));
  psyassert(!ec);
}
