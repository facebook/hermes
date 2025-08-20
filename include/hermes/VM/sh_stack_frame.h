/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "hermes/VMLayouts/sh_stack_frame_layout.h"
#include "sh_legacy_value.h"

#include <stdint.h>

#define SH_DEFINE_STACKFRAME_PTR(name)                      \
  static inline SHLegacyValue *_sh_stackframe_##name##_ptr( \
      SHLegacyValue *frame) {                               \
    return &frame[SHStackFrame_##name];                     \
  }

// Declare convenience accessors to the underlying SHLegacyValue slots.
SH_DEFINE_STACKFRAME_PTR(FirstLocal)
SH_DEFINE_STACKFRAME_PTR(DebugEnvironment)
SH_DEFINE_STACKFRAME_PTR(PreviousFrame)
SH_DEFINE_STACKFRAME_PTR(SavedIP)
SH_DEFINE_STACKFRAME_PTR(SavedCodeBlock)
SH_DEFINE_STACKFRAME_PTR(SHLocals)
SH_DEFINE_STACKFRAME_PTR(ArgCount)
SH_DEFINE_STACKFRAME_PTR(NewTarget)
SH_DEFINE_STACKFRAME_PTR(CalleeClosureOrCB)
SH_DEFINE_STACKFRAME_PTR(ThisArg)
SH_DEFINE_STACKFRAME_PTR(FirstArg)

#undef SH_DEFINE_STACKFRAME_PTR

/// \return a pointer to the register at the start of the previous stack
/// frame.
static inline SHLegacyValue *_sh_stackframe_get_previous_frame_pointer(
    SHLegacyValue *frame) {
  return (SHLegacyValue *)_sh_ljs_get_native_pointer(
      *_sh_stackframe_PreviousFrame_ptr(frame));
}

/// \return the ArgCount of the given stack frame.
static inline uint32_t _sh_stackframe_get_argcount(SHLegacyValue *frame) {
  return _sh_ljs_get_native_uint32(*_sh_stackframe_ArgCount_ptr(frame));
}

/// \return a reference to the register containing the N-th argument to the
/// callee. -1 is this, 0 is the first explicit argument. It is an error to
/// use a number greater or equal to \c _sh_stackframe_get_argcount().
static inline SHLegacyValue *_sh_stackframe_get_arg_ptr(
    SHLegacyValue *frame,
    int32_t n) {
  return &_sh_stackframe_ThisArg_ptr(frame)[-n - 1];
}
