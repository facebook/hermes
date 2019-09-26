/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/CheckHeapWellFormedAcceptor.h"

#include "hermes/VM/GC.h"

namespace hermes {
namespace vm {

#ifdef HERMES_SLOW_DEBUG

void CheckHeapWellFormedAcceptor::accept(void *&ptr) {
  assert(
      (!ptr || gc.validPointer(ptr)) &&
      "A pointer is pointing outside of the valid region");
}

void CheckHeapWellFormedAcceptor::acceptWeak(void *&ptr) {
  // A weak pointer has the same well-formed-ness checks as a normal pointer.
  accept(ptr);
}

void CheckHeapWellFormedAcceptor::accept(HermesValue &hv) {
  assert(!hv.isInvalid() && "HermesValue with InvalidTag encountered by GC.");
  if (hv.isPointer()) {
    void *cell = hv.getPointer();
    accept(cell);
  }
}

void CheckHeapWellFormedAcceptor::accept(WeakRefBase &wr) {
  // Cannot check if the weak ref is valid, since it is allowed to mark an
  // empty weak ref.
  const WeakRefSlot *slot = wr.unsafeGetSlot();
  // If the weak value is a pointer, check that it's within the valid region.
  if (slot->state() != WeakSlotState::Free && slot->hasPointer()) {
    void *cell = slot->getPointer();
    accept(cell);
  }
}

#endif

} // namespace vm
} // namespace hermes
