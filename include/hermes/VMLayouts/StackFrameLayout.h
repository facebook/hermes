/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// This file defines the standard layout of the HermesVM stack frame. It must
/// be shared between the compiler and the VM.
//===----------------------------------------------------------------------===//
#ifndef HERMES_BCGEN_HBC_STACKFRAMELAYOUT_H
#define HERMES_BCGEN_HBC_STACKFRAMELAYOUT_H

#include "hermes/VMLayouts/sh_stack_frame_layout.h"

#include <cstdint>

namespace hermes {
namespace hbc {

/// See sh_stack_frame_layout.h for the layout of the stack frame.
struct StackFrameLayout {
  enum {
  /// Mirror the fields without the SHStackFrame_ prefix.
#define FRAME_FIELD(name) name = SHStackFrame_##name
    FRAME_FIELD(FirstLocal),
    FRAME_FIELD(DebugEnvironment),
    FRAME_FIELD(PreviousFrame),
    FRAME_FIELD(SavedIP),
    FRAME_FIELD(SavedCodeBlock),
    FRAME_FIELD(SHLocals),
    FRAME_FIELD(ArgCount),
    FRAME_FIELD(NewTarget),
    FRAME_FIELD(CalleeClosureOrCB),
    FRAME_FIELD(ThisArg),
    FRAME_FIELD(FirstArg),
    FRAME_FIELD(CallerExtraRegistersAtEnd),
    FRAME_FIELD(CalleeExtraRegistersAtStart),
#undef FRAME_FIELD
  };

  /// Calculate the number of register slots needed for an outgoing call: it
  /// contains the outgoing arguments and the metadata. This saturates: on
  /// overflow it returns UINT32_MAX. Note that an overflow return is
  /// conceptually "too small" in that the true number of registers required
  /// would exceed the capacity of a uint32. The assumption is that the Runtime
  /// bounds the register stack max size below UINT32_MAX, and therefore will
  /// fail to allocate this "too small" size.
  /// \param numArgsExcludingThis number of arguments excluding \c thisArg
  /// \return the number of registers needed, or UINT32_MAX if the count
  /// would overflow.
  static uint32_t callerOutgoingRegisters(uint32_t numArgsExcludingThis) {
    // The >= pattern is specially recognized as an overflow check.
    uint32_t totalCount = CallerExtraRegistersAtEnd + numArgsExcludingThis + 1;
    return totalCount >= numArgsExcludingThis ? totalCount : UINT32_MAX;
  }
};

} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_STACKFRAMELAYOUT_H
