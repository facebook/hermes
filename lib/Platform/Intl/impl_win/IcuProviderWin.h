/*
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 */

#ifndef HERMES_PLATFORM_INTL_ICU_PROVIDER_WIN_H
#define HERMES_PLATFORM_INTL_ICU_PROVIDER_WIN_H

#include "hermes/Platform/Intl/hermes_icu.h"

namespace hermes {
namespace platform_intl {

/// Try to load Windows built-in ICU (icu.dll, available since Windows 10 1903).
/// Returns a pointer to a static hermes_icu_vtable populated via
/// GetProcAddress, or nullptr if icu.dll is not available.
/// Thread-safe: uses std::call_once internally.
const hermes_icu_vtable *getWinIcuVtable();

} // namespace platform_intl
} // namespace hermes

#endif // HERMES_PLATFORM_INTL_ICU_PROVIDER_WIN_H
