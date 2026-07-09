#include <cassert>
#include <filesystem>
#include <fstream>
#include <queue>
#include <string>
#include <string_view>

int main() {
  namespace fs = std::filesystem;

  fs::path p1("/tmp/some/dir/file.txt");
  assert(p1.filename() == fs::path("file.txt"));
  assert(p1.stem() == fs::path("file"));
  assert(p1.extension() == fs::path(".txt"));
  assert(p1.parent_path() == fs::path("/tmp/some/dir"));
  assert(p1.string() == "/tmp/some/dir/file.txt");

  fs::path p2 = std::string("noext");
  assert(p2.extension().empty());
  assert(p2.stem() == fs::path("noext"));

  fs::path joined = fs::path("/tmp/some") / "dir" / fs::path("file.txt");
  assert(joined == p1);

  std::u8string u8 = p1.u8string();
  fs::path from_u8{std::u8string_view(u8)};
  assert(from_u8 == p1);

  std::queue<fs::path> q;
  q.push(std::string("/tmp/a"));
  q.push(std::string("/tmp/b"));
  assert(q.front().string() == "/tmp/a");

  const char* tmp_name = "psychicstd_test_filesystem_tmp.txt";
  {
    std::ofstream out(tmp_name);
    out << "hi";
  }
  fs::path existing(tmp_name);
  assert(fs::exists(existing));
  assert(fs::is_regular_file(existing));
  assert(!fs::is_directory(existing));
  assert(fs::file_size(existing) == 2);
  assert(!fs::exists(fs::path("does_not_exist_hopefully.txt")));
  std::remove(tmp_name);
}
