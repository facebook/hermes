/*
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 */

#ifndef HERMES_PLATFORM_INTL_ICU_PROVIDER_DYNAMIC_H
#define HERMES_PLATFORM_INTL_ICU_PROVIDER_DYNAMIC_H

#include "hermes/Platform/Intl/hermes_icu.h"

namespace hermes {
namespace platform_intl {

/// Load ICU from user-specified DLL paths with versioned symbol names.
/// Returns 0 on success, non-zero on failure.
/// Sets the vtable as the active provider on success.
int32_t loadDynamicIcu(
    const wchar_t *icuCommonDllPath,
    const wchar_t *icuI18nDllPath,
    uint32_t icuVersion);

} // namespace platform_intl
} // namespace hermes

#endif // HERMES_PLATFORM_INTL_ICU_PROVIDER_DYNAMIC_H
