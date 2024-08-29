/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "hermes/VM/sh_legacy_value.h"

typedef struct SHRuntime SHRuntime;
typedef struct SHRuntimeModule SHRuntimeModule;

#ifdef __cplusplus
extern "C" {
#endif

/// Create a JSFunction closure. The properties of the function (name, etc.)
/// will be populated lazily.
SHERMES_EXPORT SHLegacyValue _sh_ljs_create_bytecode_closure(
    SHRuntime *shr,
    const SHLegacyValue *env,
    SHRuntimeModule *shRuntimeModule,
    uint32_t functionID);

/// Get the string associated with the given RuntimeModule-specific string ID.
/// This may lazily allocate a SymbolID and the string itself.
/// This is used when executing with a bytecode CodeBlock.
SHERMES_EXPORT SHLegacyValue _sh_ljs_get_bytecode_string(
    SHRuntime *shr,
    SHRuntimeModule *runtimeModule,
    uint32_t stringID);

#ifdef __cplusplus
}
#endif
