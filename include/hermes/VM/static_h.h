/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_STATIC_H_H
#define HERMES_STATIC_H_H

#include "hermes/VM/sh_legacy_value.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SHRuntime SHRuntime;

/// Create a runtime instance.
SHRuntime *_sh_init(void);
/// Destroy a runtime instance created by \c _sh_init();
void _sh_done(SHRuntime *shr);

#ifdef __cplusplus
}
#endif

#endif // HERMES_STATIC_H_H
