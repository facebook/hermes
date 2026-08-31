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

#include <memory>

namespace hermes {
namespace vm {

class Runtime;
class NativeState;
using FinalizeNativeStatePtr = void (*)(GC &gc, NativeState *ns);

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

  /// Create a new NativeState on the JS heap which shares ownership of \p
  /// context with every other holder of that shared pointer. Unlike a
  /// NativeState created by create(), such a NativeState can be duplicated
  /// into another Runtime, because the duplicate shares ownership of the same
  /// native data. This is used by the structured clone algorithm, see
  /// SerializedValue.h.
  static NativeState *createShared(
      Runtime &runtime,
      std::shared_ptr<void> context);

  /// \return true if this NativeState was created by createShared, and its
  /// context can therefore be shared with another NativeState.
  bool isShared() const {
    return finalizePtr_ == _finalizeSharedImpl;
  }

  /// \return the shared context owned by this NativeState.
  /// \pre isShared() must be true.
  const std::shared_ptr<void> &getSharedContext() const {
    assert(isShared() && "NativeState does not own a shared context");
    return *static_cast<const std::shared_ptr<void> *>(context_);
  }

  void *context() {
    return context_;
  }
  void setContext(void *context) {
    context_ = context;
  }

 private:
  static void _finalizeImpl(GCCell *cell, GC &gc);

  /// Finalizer for a NativeState created by createShared. It doubles as the tag
  /// that identifies such a NativeState, see isShared().
  static void _finalizeSharedImpl(GC &gc, NativeState *ns);

  void *context_;
  FinalizeNativeStatePtr finalizePtr_;
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_NATIVESTATE_H
