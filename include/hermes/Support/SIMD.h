/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/// Feature detection macros for SIMD instruction sets.
/// Include this header to get HERMES_SIMD_NEON or HERMES_SIMD_SSE2 defined
/// (if available), along with the corresponding intrinsics header.

#pragma once

// Determine which SIMD backend to use.
#if (defined(__GNUC__) && defined(__aarch64__)) || \
    (defined(_MSC_VER) && defined(_M_ARM64))
#define HERMES_SIMD_NEON
#include <arm_neon.h>
#elif (defined(__GNUC__) && defined(__x86_64__)) || \
    (defined(_MSC_VER) && defined(_M_X64))
#define HERMES_SIMD_SSE2
#include <emmintrin.h>
#endif
