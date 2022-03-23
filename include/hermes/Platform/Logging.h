/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PLATFORM_LOGGING_H
#define HERMES_PLATFORM_LOGGING_H

#include "hermes/Support/Compiler.h"

namespace hermes {

/// hermesLog does logging in the current platform's standard way,
/// such as stderr, logcat or console.
void hermesLog(const char *componentName, const char *fmt, ...)
    HERMES_ATTRIBUTE_FORMAT(printf, 2, 3);

} // namespace hermes

#endif // HERMES_PLATFORM_LOGGING_H
