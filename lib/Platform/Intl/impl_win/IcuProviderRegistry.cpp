/*
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 */

/**
 * ICU provider registry - selects and caches the active ICU provider.
 *
 * Priority:
 *   1. Host-provided vtable (via hermes_icu_set_vtable)
 *   2. Bundled hermes-icu.dll (same directory as hermes.dll)
 *   3. Windows built-in icu.dll (1903+)
 *   4. nullptr (caller falls back to WinGlob NLS APIs)
 */

#include "IcuProviderRegistry.h"
#include "IcuProviderBundled.h"
#include "IcuProviderDynamic.h"
#include "IcuProviderWin.h"

#include "hermes/Platform/Intl/IntlProviderInfo.h"
#include "hermes/VM/Runtime.h"

#include <atomic>
#include <mutex>

namespace hermes {
namespace platform_intl {
namespace {

/// Host-provided vtable, set via hermes_icu_set_vtable().
/// Must be set before any runtime is created.
static std::atomic<const hermes_icu_vtable *> s_hostVtable{nullptr};

/// Cached resolved vtable (after auto-detection).
static const hermes_icu_vtable *s_resolvedVtable = nullptr;
/// Cached provider name (set during resolution).
static const char *s_resolvedProviderName = "none";
static std::once_flag s_resolveFlag;

static void resolveVtable() {
  // 1. Host-provided vtable takes priority.
  const hermes_icu_vtable *host =
      s_hostVtable.load(std::memory_order_acquire);
  if (host) {
    s_resolvedVtable = host;
    s_resolvedProviderName =
        host->provider_name ? host->provider_name : "custom";
    return;
  }

  // 2. Try bundled hermes-icu.dll (same directory as hermes.dll).
  const hermes_icu_vtable *bundled = getBundledIcuVtable();
  if (bundled) {
    s_resolvedVtable = bundled;
    s_resolvedProviderName = "bundled";
    return;
  }

  // 3. Try Windows built-in icu.dll (1903+).
  const hermes_icu_vtable *win = getWinIcuVtable();
  if (win) {
    s_resolvedVtable = win;
    s_resolvedProviderName = "windows";
    return;
  }

  // 4. No ICU available - caller should fall back to WinGlob.
  s_resolvedVtable = nullptr;
  s_resolvedProviderName = "winglob";
}

} // namespace

const hermes_icu_vtable *getActiveIcuVtable() {
  // Fast path: if host set a vtable, return it directly (no caching needed,
  // the host vtable is set once before any runtime is created).
  const hermes_icu_vtable *host =
      s_hostVtable.load(std::memory_order_acquire);
  if (host)
    return host;

  // Slow path: auto-detect on first call.
  std::call_once(s_resolveFlag, resolveVtable);
  return s_resolvedVtable;
}

const char *getActiveIcuProviderName() {
  // Fast path: host vtable bypasses cached resolution.
  const hermes_icu_vtable *host =
      s_hostVtable.load(std::memory_order_acquire);
  if (host)
    return host->provider_name ? host->provider_name : "custom";

  // Trigger resolution if not done yet, then return cached name.
  std::call_once(s_resolveFlag, resolveVtable);
  return s_resolvedProviderName;
}

uint32_t getActiveIcuVersion() {
  const hermes_icu_vtable *vt = getActiveIcuVtable();
  return vt ? vt->icu_version : 0;
}

IntlProviderInfo getIntlProviderInfo() {
  return {getActiveIcuProviderName(), getActiveIcuVersion()};
}

const hermes_icu_vtable *getIcuVtableForRuntime(vm::Runtime &runtime) {
  uint8_t mode = runtime.getIntlProviderMode();
  if (mode == static_cast<uint8_t>(IntlProviderMode::ForceWinGlob))
    return nullptr;
  if (mode == static_cast<uint8_t>(IntlProviderMode::CustomVtable))
    return static_cast<const hermes_icu_vtable *>(
        runtime.getIntlIcuVtable());
  // Default: use global resolution.
  return getActiveIcuVtable();
}

IntlProviderInfo getIntlProviderInfoForRuntime(vm::Runtime &runtime) {
  uint8_t mode = runtime.getIntlProviderMode();
  if (mode == static_cast<uint8_t>(IntlProviderMode::ForceWinGlob))
    return {"winglob", 0};
  if (mode == static_cast<uint8_t>(IntlProviderMode::CustomVtable)) {
    auto *vt = static_cast<const hermes_icu_vtable *>(
        runtime.getIntlIcuVtable());
    if (!vt)
      return {"winglob", 0};
    return {vt->provider_name ? vt->provider_name : "custom",
            vt->icu_version};
  }
  // Default: use global info.
  return getIntlProviderInfo();
}

IntlProviderInfo getIntlProviderInfo(vm::Runtime &runtime) {
  return getIntlProviderInfoForRuntime(runtime);
}

} // namespace platform_intl
} // namespace hermes

// ============================================================================
// C API implementations (declared in hermes_icu.h)
// ============================================================================

extern "C" {

void HERMES_ICU_CDECL hermes_icu_set_vtable(const hermes_icu_vtable *vt) {
  hermes::platform_intl::s_hostVtable.store(vt, std::memory_order_release);
}

const hermes_icu_vtable *HERMES_ICU_CDECL hermes_icu_get_active_vtable(void) {
  return hermes::platform_intl::getActiveIcuVtable();
}

#ifdef _WIN32
HERMES_ICU_API int32_t HERMES_ICU_CDECL hermes_icu_load_from_path(
    const wchar_t *icu_common_dll_path,
    const wchar_t *icu_i18n_dll_path,
    uint32_t icu_version) {
  return hermes::platform_intl::loadDynamicIcu(
      icu_common_dll_path, icu_i18n_dll_path, icu_version);
}
#endif

} // extern "C"
