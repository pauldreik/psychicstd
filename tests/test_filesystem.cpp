#include "psyassert.h"
#include <filesystem>
#include <fstream>
#include <queue>
#include <sstream>
#include <string>
#include <string_view>
#include <sys/stat.h>
#include <unistd.h>

int main() {
  namespace fs = std::filesystem;

  fs::path p1("/tmp/some/dir/file.txt");
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

  fs::path p2 = std::string("noext");
  psyassert(p2.extension().empty());
  psyassert(p2.stem() == fs::path("noext"));

  fs::path joined = fs::path("/tmp/some") / "dir" / fs::path("file.txt");
  psyassert(joined == p1);
  psyassert(p1.lexically_relative("/tmp/some") == fs::path("dir/file.txt"));
  psyassert(p1.lexically_relative(p1) == fs::path("."));
  psyassert(fs::path("archive.tar.gz").replace_extension() ==
            fs::path("archive.tar"));
  psyassert(fs::path("archive.tar.gz").replace_extension("txt") ==
            fs::path("archive.tar.txt"));

  std::u8string u8 = p1.u8string();
  fs::path from_u8{std::u8string_view(u8)};
  psyassert(from_u8 == p1);

  std::queue<fs::path> q;
  q.push(std::string("/tmp/a"));
  q.push(std::string("/tmp/b"));
  psyassert(q.front().string() == "/tmp/a");

  const char* tmp_name = "psychicstd_test_filesystem_tmp.txt";
  {
    std::ofstream out(tmp_name);
    out << "hi";
  }
  fs::path existing(tmp_name);
  psyassert(fs::exists(existing));
  psyassert(fs::is_regular_file(existing));
  psyassert(!fs::is_directory(existing));
  psyassert(fs::file_size(existing) == 2);
  psyassert(!fs::exists(fs::path("does_not_exist_hopefully.txt")));
  std::remove(tmp_name);

  const char* tree = "psychicstd_test_filesystem_tree";
  const char* subtree = "psychicstd_test_filesystem_tree/sub";
  const char* child = "psychicstd_test_filesystem_tree/sub/file.txt";
  ::mkdir(tree, 0700);
  ::mkdir(subtree, 0700);
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
  std::remove(child);
  ::rmdir(subtree);
  ::rmdir(tree);
}
