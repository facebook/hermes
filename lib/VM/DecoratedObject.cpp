/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/DecoratedObject.h"

#include "hermes/VM/Runtime-inline.h"
#pragma GCC diagnostic push

#ifdef HERMES_COMPILER_SUPPORTS_WSHORTEN_64_TO_32
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif
namespace hermes {
namespace vm {
//===----------------------------------------------------------------------===//
// class DecoratedObject

size_t DecoratedObject::Decoration::getMallocSize() const {
  return sizeof *this;
}

const ObjectVTable DecoratedObject::vt{
    VTable(
        CellKind::DecoratedObjectKind,
        cellSize<DecoratedObject>(),
        DecoratedObject::_finalizeImpl,
        nullptr, /* markWeak */
        DecoratedObject::_mallocSizeImpl),
    DecoratedObject::_getOwnIndexedRangeImpl,
    DecoratedObject::_haveOwnIndexedImpl,
    DecoratedObject::_getOwnIndexedPropertyFlagsImpl,
    DecoratedObject::_getOwnIndexedImpl,
    DecoratedObject::_setOwnIndexedImpl,
    DecoratedObject::_deleteOwnIndexedImpl,
    DecoratedObject::_checkAllOwnIndexedImpl,
};

void DecoratedObjectBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<DecoratedObject>());
  JSObjectBuildMeta(cell, mb);
  mb.setVTable(&DecoratedObject::vt);
}

// static
PseudoHandle<DecoratedObject> DecoratedObject::create(
    Runtime &runtime,
    Handle<JSObject> parentHandle,
    std::unique_ptr<Decoration> decoration,
    unsigned int additionalSlotCount) {
  const size_t reservedSlots =
      numOverlapSlots<DecoratedObject>() + additionalSlotCount;
  auto *cell = runtime.makeAFixed<DecoratedObject, HasFinalizer::Yes>(
      runtime,
      parentHandle,
      runtime.getHiddenClassForPrototype(*parentHandle, reservedSlots),
      std::move(decoration));
  auto self = JSObjectInit::initToPseudoHandle(runtime, cell);
  // Allocate a propStorage if the number of additional slots requires it.
  auto selfWithSlots = runtime.ignoreAllocationFailure(
      JSObject::allocatePropStorage(std::move(self), runtime, reservedSlots));
  return PseudoHandle<DecoratedObject>::vmcast(std::move(selfWithSlots));
}

// static
void DecoratedObject::_finalizeImpl(GCCell *cell, GC &) {
  auto *self = vmcast<DecoratedObject>(cell);
  self->~DecoratedObject();
}

// static
size_t DecoratedObject::_mallocSizeImpl(GCCell *cell) {
  DecoratedObject *self = vmcast<DecoratedObject>(cell);
  if (const Decoration *deco = self->getDecoration()) {
    return deco->getMallocSize();
  }
  return 0;
}

} // namespace vm
} // namespace hermes
