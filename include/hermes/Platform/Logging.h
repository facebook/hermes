/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_PLATFORM_LOGGING_H
#define HERMES_PLATFORM_LOGGING_H

#include "hermes/Support/Compiler.h"

namespace hermes {

/// hermesLog does logging in a platform-dependent way:
/// * On Android, writes to logcat.
/// * On iOS, currently does nothing.
/// * On Linux, writes to stderr.
void hermesLog(const char *componentName, const char *fmt, ...)
    HERMES_ATTRIBUTE_FORMAT(printf, 2, 3);

} // namespace hermes

#endif // HERMES_PLATFORM_LOGGING_H
