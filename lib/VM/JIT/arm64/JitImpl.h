/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "asmjit/a64.h"

namespace hermes::vm::arm64 {

class JITContext::Impl {
 public:
  asmjit::JitRuntime jr{};

  /// Previous HC ID assigned to a hidden class when we need to permanently tag
  /// it.
  uint16_t prevHCId{0};

  static constexpr uint16_t kHCIdOverflow = 0xFFFF;

  /// The largest id assignable to a hidden class. Lowered only by tests, so
  /// that the exhaustion path can be reached without interning 65535 hidden
  /// classes; the true ceiling is the width of HiddenClass::lazyJITId_.
  uint16_t hcIdLimit{kHCIdOverflow};

  /// Hidden classes that we used by the emitted JIT code. Since we never
  /// throw away code, they can never be freed.
  /// This value is either Undefined or  a pointer to ArrayStorageSmall. It is
  /// convenient to keep it as a PHV, so that it can also be used as a handle.
  PinnedHermesValue usedHCs = HermesValue::encodeUndefinedValue();
};

}; // namespace hermes::vm::arm64
