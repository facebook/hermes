/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_SLOWASSERT_H
#define HERMES_SUPPORT_SLOWASSERT_H

#include <cassert>

/// HERMES_SLOW_ASSERT and SLOW_DEBUG are variants of assert() and LLVM_DEBUG()
/// respectively which are enabled only if HERMES_SLOW_DEBUG is defined. This is
/// useful for checks which may be expensive.
#ifdef HERMES_SLOW_DEBUG
#define HERMES_SLOW_ASSERT(x) assert(x)
#define SLOW_DEBUG(x) LLVM_DEBUG(x)
#else
#define HERMES_SLOW_ASSERT(x) ((void)0)
#define SLOW_DEBUG(x) \
  do {                \
  } while (false)
#endif

#endif // HERMES_SUPPORT_SLOWASSERT_H
