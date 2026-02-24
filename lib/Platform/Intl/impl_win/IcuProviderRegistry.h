/*
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 */

#ifndef HERMES_PLATFORM_INTL_ICU_PROVIDER_REGISTRY_H
#define HERMES_PLATFORM_INTL_ICU_PROVIDER_REGISTRY_H

#include "hermes/Platform/Intl/hermes_icu.h"

namespace hermes {
namespace vm {
class Runtime;
} // namespace vm

namespace platform_intl {

struct IntlProviderInfo;

/// Get the active ICU vtable, selecting from available providers (global).
///
/// Priority order:
///   1. Host-provided vtable (via hermes_icu_set_vtable)
///   2. Bundled hermes-icu.dll (same directory as hermes.dll)
///   3. Windows built-in ICU (icu.dll, 1903+)
///   4. nullptr (caller should fall back to WinGlob NLS)
///
/// Thread-safe. The result is cached after first resolution.
const hermes_icu_vtable *getActiveIcuVtable();

/// Get the ICU vtable for a specific runtime.
///
/// Respects per-runtime IntlProviderMode:
///   - Default (0): fall back to global getActiveIcuVtable()
///   - ForceWinGlob (1): return nullptr
///   - CustomVtable (2): return the per-runtime vtable
const hermes_icu_vtable *getIcuVtableForRuntime(vm::Runtime &runtime);

/// Get the name of the active ICU provider.
/// Returns "bundled", "windows", "custom", "winglob", or "none".
/// Triggers lazy resolution if not already resolved.
const char *getActiveIcuProviderName();

/// Get the ICU version of the active provider.
/// Returns 0 if no ICU provider is active.
uint32_t getActiveIcuVersion();

/// Get provider info for a specific runtime.
IntlProviderInfo getIntlProviderInfoForRuntime(vm::Runtime &runtime);

} // namespace platform_intl
} // namespace hermes

#endif // HERMES_PLATFORM_INTL_ICU_PROVIDER_REGISTRY_H
