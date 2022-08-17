/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/NativeState.h"

#include "hermes/VM/Runtime.h"

namespace hermes {
namespace vm {

const VTable NativeState::vt{
    CellKind::NativeStateKind,
    cellSize<NativeState>(),
    _finalizeImpl,
};

void NativeStateBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.setVTable(&NativeState::vt);
}

/* static */
NativeState *NativeState::create(
    Runtime &runtime,
    void *context,
    FinalizeNativeStatePtr finalizePtr) {
  return runtime.makeAFixed<NativeState, HasFinalizer::Yes>(
      context, finalizePtr);
}

void NativeState::_finalizeImpl(GCCell *cell, GC &gc) {
  auto *self = vmcast<NativeState>(cell);
  self->~NativeState();
}

} // namespace vm
} // namespace hermes
