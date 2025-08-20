/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SH_CONFIG_H
#define HERMES_SH_CONFIG_H

#include "libhermesvm-config.h"

/// Number of property slots allocated directly inside the object.
enum { HERMESVM_DIRECT_PROPERTY_SLOTS = 5 };

#ifndef __cplusplus
// uchar.h is not universally available, so just define our own.
typedef uint16_t char16_t;
#endif

#ifndef SHERMES_EXPORT
#ifdef _MSC_VER
#define SHERMES_EXPORT __declspec(dllexport)
#else // _MSC_VER
#define SHERMES_EXPORT __attribute__((visibility("default")))
#endif // _MSC_VER
#endif // !defined(HERMES_EXPORT)

/// \macro SHERMES_NO_SANITIZE
/// Disable a particular sanitizer for a function.
#if __has_attribute(no_sanitize)
#define SHERMES_NO_SANITIZE(KIND) __attribute__((no_sanitize(KIND)))
#else
#define SHERMES_NO_SANITIZE(KIND)
#endif

#ifdef __GNUC__
#define SH_LIKELY(EXPR) __builtin_expect((bool)(EXPR), true)
#define SH_UNLIKELY(EXPR) __builtin_expect((bool)(EXPR), false)
#else
#define SH_LIKELY(EXPR) (EXPR)
#define SH_UNLIKELY(EXPR) (EXPR)
#endif

/// SH_ATTRIBUTE_NOINLINE - On compilers where we have a directive to do so,
/// mark a method "not for inlining".
#ifdef __GNUC__
#define SH_ATTRIBUTE_NOINLINE __attribute__((noinline))
#else
#define SH_ATTRIBUTE_NOINLINE
#endif

/// SH_ATTRIBUTE_ALWAYS_INLINE - On compilers where we have a directive to do
/// so, mark a method "always inline" because it is performance sensitive.
#ifdef __GNUC__
#define SH_ATTRIBUTE_ALWAYS_INLINE __attribute__((always_inline))
#else
#define SH_ATTRIBUTE_ALWAYS_INLINE
#endif

#endif // HERMES_SH_CONFIG_H
