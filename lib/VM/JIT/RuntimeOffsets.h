/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JIT_X86_64_RUNTIMEOFFSETS_H
#define HERMES_VM_JIT_X86_64_RUNTIMEOFFSETS_H

#include "hermes/VM/Runtime.h"

namespace hermes {
namespace vm {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"

struct RuntimeOffsets {
  static constexpr uint32_t stackPointer = offsetof(Runtime, stackPointer_);
  static constexpr uint32_t currentFrame = offsetof(Runtime, currentFrame_);
  static constexpr uint32_t globalObject = offsetof(Runtime, global_);
  static constexpr uint32_t thrownValue = offsetof(Runtime, thrownValue_);
  static constexpr uint32_t shLocals = offsetof(Runtime, shLocals);
  static constexpr uint32_t nativeStackHigh =
      offsetof(Runtime, overflowGuard_) +
      offsetof(StackOverflowGuard, nativeStackHigh);
  static constexpr uint32_t nativeStackSize =
      offsetof(Runtime, overflowGuard_) +
      offsetof(StackOverflowGuard, nativeStackSize);
};

#pragma GCC diagnostic pop

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_JIT_X86_64_RUNTIMEOFFSETS_H
