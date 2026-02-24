/*
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 */

/**
 * Bundled ICU provider.
 *
 * Loads hermes-icu.dll from the same directory as the calling module
 * (hermes.dll) via LoadLibraryExW. This ensures we load our bundled
 * ICU rather than picking up a random DLL from the PATH.
 *
 * Security: Uses LOAD_WITH_ALTERED_SEARCH_PATH with a fully qualified
 * path derived from GetModuleFileName on the current module.
 */

#include "IcuProviderBundled.h"

#include <mutex>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace hermes {
namespace platform_intl {
namespace {

static const hermes_icu_vtable *s_bundledVtable = nullptr;
static std::once_flag s_bundledInitFlag;
static bool s_bundledLoaded = false;

static bool loadBundledIcuDll() {
  // Find the directory of the current module (hermes.dll).
  // We use GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS to find the module
  // that contains this function, which is hermes.dll.
  HMODULE hSelf = nullptr;
  if (!GetModuleHandleExW(
          GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
              GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
          reinterpret_cast<LPCWSTR>(&loadBundledIcuDll),
          &hSelf)) {
    return false;
  }

  wchar_t path[MAX_PATH];
  DWORD pathLen = GetModuleFileNameW(hSelf, path, MAX_PATH);
  if (pathLen == 0 || pathLen >= MAX_PATH)
    return false;

  // Replace the filename portion with "hermes-icu.dll".
  wchar_t *lastSep = wcsrchr(path, L'\\');
  if (!lastSep)
    return false;

  // Check we have enough room for "hermes-icu.dll" + null.
  size_t dirLen = static_cast<size_t>(lastSep - path + 1);
  const wchar_t kDllName[] = L"hermes-icu.dll";
  if (dirLen + wcslen(kDllName) >= MAX_PATH)
    return false;

  wcscpy_s(lastSep + 1, MAX_PATH - dirLen, kDllName);

  // Load hermes-icu.dll from the computed path.
  // LOAD_WITH_ALTERED_SEARCH_PATH ensures DLL dependencies are searched
  // relative to the loaded DLL's directory.
  HMODULE dll = LoadLibraryExW(path, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
  if (!dll)
    return false;

  // Resolve the single export.
  // NOLINTNEXTLINE: GetProcAddress returns FARPROC which needs a cast.
  auto getVtable = reinterpret_cast<hermes_icu_get_vtable_fn>(
      reinterpret_cast<void *>(
          GetProcAddress(dll, "hermes_icu_get_vtable")));
  if (!getVtable) {
    FreeLibrary(dll);
    return false;
  }

  const hermes_icu_vtable *vt = getVtable();
  if (!vt || vt->version != HERMES_ICU_VTABLE_VERSION) {
    FreeLibrary(dll);
    return false;
  }

  s_bundledVtable = vt;
  // Intentionally leak the DLL handle — it stays loaded for the process
  // lifetime, matching the pattern used by IcuProviderWin.
  return true;
}

} // namespace

const hermes_icu_vtable *getBundledIcuVtable() {
  std::call_once(s_bundledInitFlag, []() {
    s_bundledLoaded = loadBundledIcuDll();
  });
  return s_bundledLoaded ? s_bundledVtable : nullptr;
}

} // namespace platform_intl
} // namespace hermes
