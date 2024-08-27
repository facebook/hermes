/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JIT_CONFIG_H
#define HERMES_VM_JIT_CONFIG_H

#ifdef HERMESVM_JIT
#error HERMESVM_JIT must only be defined by this file
#endif

#ifndef HERMESVM_ALLOW_JIT
#define HERMESVM_JIT 0
#elif HERMESVM_ALLOW_JIT == 2
#define HERMESVM_JIT 1
#elif HERMESVM_ALLOW_JIT == 1

// Disable JIT on Apple platforms that prohibit it.
#ifdef __APPLE__
#include <TargetConditionals.h>
#if !defined(HERMESVM_JIT) &&                                      \
    !((defined(TARGET_OS_MACCATALYST) && TARGET_OS_MACCATALYST) || \
      (defined(TARGET_OS_OSX) && TARGET_OS_OSX))
#define HERMESVM_JIT 0
#endif
#endif

// If the JIT is allowed by configuration, enable it on platforms that support
// it.
#if !defined(HERMESVM_JIT) && (defined(__aarch64__) || defined(_M_ARM64))
#define HERMESVM_JIT 1
#else
#define HERMESVM_JIT 0
#endif

#else
#error HERMESVM_ALLOW_JIT must have a value of 1 or 2
#endif

#endif // HERMES_VM_JIT_CONFIG_H
