#pragma once
#include <windows.h>
#include <cstring>
#include <string>

namespace xyl::util {

inline std::string readFile(const char *path) {
  HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
  if (h == INVALID_HANDLE_VALUE) return {};
  LARGE_INTEGER sz; GetFileSizeEx(h, &sz);
  std::string buf((size_t)sz.QuadPart, '\0');
  DWORD rd = 0; ReadFile(h, buf.data(), (DWORD)buf.size(), &rd, nullptr);
  CloseHandle(h);
  buf.resize(rd);
  return buf;
}

inline bool writeFile(const char *path, const std::string &data) {
  HANDLE h = CreateFileA(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (h == INVALID_HANDLE_VALUE) return false;
  DWORD wr = 0; BOOL ok = WriteFile(h, data.data(), (DWORD)data.size(), &wr, nullptr);
  CloseHandle(h);
  return ok && wr == data.size();
}

inline std::string moduleDir(HMODULE module) {
  char path[MAX_PATH] = {0};
  if (!GetModuleFileNameA(module, path, MAX_PATH)) return {};
  char *slash = strrchr(path, '\\');
  if (slash) *slash = '\0';
  return path;
}

} // namespace xyl::util
