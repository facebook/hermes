/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_NATIVESTATE_H
#define HERMES_VM_NATIVESTATE_H

#include "hermes/VM/GCCell.h"
#include "hermes/VM/Metadata.h"

namespace hermes {
namespace vm {

class Runtime;

using FinalizeNativeStatePtr = void (*)(void *context);

/// Wrapper for a pointer to some arbitrary native data (not on the JS heap)
/// + a function that's invoked on that data when the NativeState is finalized.
class NativeState final : public GCCell {
  friend void NativeStateBuildMeta(const GCCell *cell, Metadata::Builder &mb);

 public:
  static const VTable vt;

  static constexpr CellKind getCellKind() {
    return CellKind::NativeStateKind;
  }

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::NativeStateKind;
  }

  NativeState(void *context, FinalizeNativeStatePtr finalizePtr)
      : context_(context), finalizePtr_(finalizePtr) {
    assert(finalizePtr && "use a no-op if you are sure that is what you want");
  }

  /// Create a new NativeState on the JS heap.
  static NativeState *
  create(Runtime &runtime, void *context, FinalizeNativeStatePtr finalizePtr);

  ~NativeState() {
    finalizePtr_(context_);
  }

  void *context() {
    return context_;
  }

 private:
  static void _finalizeImpl(GCCell *cell, GC &gc);

  void *context_;
  FinalizeNativeStatePtr finalizePtr_;
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_NATIVESTATE_H
