#include <cassert>
#include <cwchar>

int main() {
  const wchar_t* a = L"hello";
  const wchar_t* b = L"help";
  assert(std::wcslen(a) == 5);
  assert(std::wcsncmp(a, b, 3) == 0);
  assert(std::wcscmp(a, b) < 0);

  wchar_t buf[16];
  std::wcscpy(buf, a);
  std::wcscat(buf, L" world");
  assert(std::wcscmp(buf, L"hello world") == 0);

  assert(std::wcschr(buf, L'w') != nullptr);
  assert(std::wcsstr(buf, L"wor") != nullptr);

  wchar_t block[5] = {L'x', L'x', L'x', L'x', L'x'};
  std::wmemset(block, L'o', 3);
  assert(block[0] == L'o' && block[3] == L'x');

  wchar_t dest[5];
  std::wmemcpy(dest, block, 5);
  assert(std::wmemcmp(dest, block, 5) == 0);
}
