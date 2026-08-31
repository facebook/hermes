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
    /* allowLargeAlloc */ false,
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

/* static */
NativeState *NativeState::createShared(
    Runtime &runtime,
    std::shared_ptr<void> context) {
  auto *box = new std::shared_ptr<void>(std::move(context));
  return create(runtime, box, _finalizeSharedImpl);
}

void NativeState::_finalizeSharedImpl(GC &, NativeState *ns) {
  delete static_cast<std::shared_ptr<void> *>(ns->context());
}

void NativeState::_finalizeImpl(GCCell *cell, GC &gc) {
  auto *self = vmcast<NativeState>(cell);
  self->finalizePtr_(gc, self);
}

} // namespace vm
} // namespace hermes
