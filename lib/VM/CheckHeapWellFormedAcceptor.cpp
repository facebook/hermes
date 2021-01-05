/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/CheckHeapWellFormedAcceptor.h"

#include "hermes/VM/GC.h"

namespace hermes {
namespace vm {

#ifdef HERMES_SLOW_DEBUG

CheckHeapWellFormedAcceptor::CheckHeapWellFormedAcceptor(GC &gc)
    : RootAndSlotAcceptorDefault(gc), WeakRootAcceptorDefault(gc) {}

void CheckHeapWellFormedAcceptor::accept(void *&ptr) {
  accept(static_cast<const void *>(ptr));
}

void CheckHeapWellFormedAcceptor::accept(const void *ptr) {
  assert(
      (!ptr || gc.validPointer(ptr)) &&
      "A pointer is pointing outside of the valid region");
}

void CheckHeapWellFormedAcceptor::acceptWeak(void *&ptr) {
  // A weak pointer has the same well-formed-ness checks as a normal pointer.
  accept(ptr);
}

void CheckHeapWellFormedAcceptor::acceptHV(HermesValue &hv) {
  assert(!hv.isInvalid() && "HermesValue with InvalidTag encountered by GC.");
  if (hv.isPointer()) {
    void *cell = hv.getPointer();
    accept(cell);
  } else if (hv.isSymbol()) {
    acceptSym(hv.getSymbol());
  }
}

void CheckHeapWellFormedAcceptor::acceptSym(SymbolID sym) {
  if (!sym.isValid()) {
    return;
  }
  assert(
      gc.getCallbacks()->isSymbolLive(sym) &&
      "Symbol is marked but is not live");
  // Check that the string used by this symbol is valid.
  accept(gc.getCallbacks()->getStringForSymbol(sym));
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
