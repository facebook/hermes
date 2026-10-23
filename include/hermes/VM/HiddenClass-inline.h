/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "hermes/VM/HiddenClass.h"

#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/JSObject.h"

namespace hermes::vm {

inline HiddenClass::HiddenClass(
    Runtime &runtime,
    ClassFlags flags,
    Handle<HiddenClass> parent,
    Handle<JSObject> objectParent,
    SymbolID symbolID,
    PropertyFlags propertyFlags,
    unsigned numProperties)
    : parent_(runtime, *parent, runtime.getHeap()),
      objectParent_(runtime, *objectParent, runtime.getHeap()),
      symbolID_(symbolID),
      propertyFlags_(propertyFlags),
      numProperties_(numProperties),
      flags_(flags) {
  assert(propertyFlags.isValid() && "propertyFlags must be valid");
}

/// \return the object's parent. May be nullptr.
inline JSObject *HiddenClass::getObjectParent(PointerBase &pb) const {
  return objectParent_.get(pb);
}

inline HiddenClass *HiddenClass::updateObjectParent(
    Handle<HiddenClass> selfHandle,
    Runtime &runtime,
    PseudoHandle<JSObject> newParentPH) {
  if (newParentPH.get() == selfHandle->objectParent_.get(runtime))
    return *selfHandle;

  return HiddenClass::updateObjectParentSlowPath(
      selfHandle, runtime, std::move(newParentPH));
}

} // namespace hermes::vm
