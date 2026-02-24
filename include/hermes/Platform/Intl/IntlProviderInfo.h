/*
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 */

/**
 * \file IntlProviderInfo.h
 * \brief Thin abstraction for querying ICU provider info from VM code.
 *
 * This avoids the VM layer depending on hermes_icu.h directly.
 */

#ifndef HERMES_PLATFORM_INTL_PROVIDER_INFO_H
#define HERMES_PLATFORM_INTL_PROVIDER_INFO_H

#include <stdint.h>

namespace hermes {
namespace vm {
class Runtime;
} // namespace vm

namespace platform_intl {

/// Per-runtime Intl provider selection mode.
enum class IntlProviderMode : uint8_t {
  /// Use global resolution (existing behavior: host > bundled > windows > winglob).
  Default = 0,
  /// Always use WinGlob NLS APIs, even if ICU is available.
  ForceWinGlob = 1,
  /// Use a per-runtime ICU vtable from RuntimeConfig.
  CustomVtable = 2,
};

/// Information about the active ICU provider.
struct IntlProviderInfo {
  /// Provider name: "bundled", "windows", "custom", "winglob", or "none".
  const char *providerName;
  /// ICU version (e.g. 78). 0 = unknown or no ICU.
  uint32_t icuVersion;
};

/// Get info about the currently active ICU provider (global).
/// Safe to call from any thread after the first Intl operation.
IntlProviderInfo getIntlProviderInfo();

/// Get info about the ICU provider for a specific runtime.
/// Respects per-runtime IntlProviderMode configuration.
IntlProviderInfo getIntlProviderInfo(vm::Runtime &runtime);

} // namespace platform_intl
} // namespace hermes

#endif // HERMES_PLATFORM_INTL_PROVIDER_INFO_H
