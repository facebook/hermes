/*
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 */

/**
 * Bundled ICU provider.
 *
 * Loads hermes-icu.dll from the same directory as hermes.dll and
 * returns the vtable via the single exported hermes_icu_get_vtable().
 */

#ifndef HERMES_ICUPROVIDER_BUNDLED_H
#define HERMES_ICUPROVIDER_BUNDLED_H

#include "hermes/Platform/Intl/hermes_icu.h"

namespace hermes {
namespace platform_intl {

/// Try to load hermes-icu.dll from the same directory as hermes.dll.
/// Returns the vtable on success, nullptr if the DLL is not found or
/// the vtable version does not match.
const hermes_icu_vtable *getBundledIcuVtable();

} // namespace platform_intl
} // namespace hermes

#endif // HERMES_ICUPROVIDER_BUNDLED_H
