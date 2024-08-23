/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "hermes/VM/sh_legacy_value.h"
#include "hermes/VM/sh_runtime.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef SHLegacyValue (*JitFn)(SHRuntime *shr);

JitFn compile_loop1(void);

#ifdef __cplusplus
}
#endif
