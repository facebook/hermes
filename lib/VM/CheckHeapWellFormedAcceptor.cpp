/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/CheckHeapWellFormedAcceptor.h"

#include "hermes/VM/GC.h"
#include "hermes/VM/SmallHermesValue-inline.h"

namespace hermes {
namespace vm {

#ifdef HERMES_SLOW_DEBUG

CheckHeapWellFormedAcceptor::CheckHeapWellFormedAcceptor(GCBase &gc) : gc(gc) {}

void CheckHeapWellFormedAcceptor::accept(GCCell *&ptr) {
  accept(static_cast<const GCCell *>(ptr));
}

void CheckHeapWellFormedAcceptor::accept(PinnedHermesValue &hv) {
  assert((!hv.isPointer() || hv.getPointer()) && "Value is not nullable.");
  acceptHV(hv);
}
void CheckHeapWellFormedAcceptor::acceptNullable(PinnedHermesValue &hv) {
  acceptHV(hv);
}
void CheckHeapWellFormedAcceptor::accept(const RootSymbolID &sym) {
  acceptSym(sym);
}
void CheckHeapWellFormedAcceptor::acceptWeak(WeakRootBase &ptr) {
  // A weak pointer has the same well-formed-ness checks as a normal pointer.
  GCCell *p = ptr.getNoBarrierUnsafe(gc.getPointerBase());
  accept(p);
}
void CheckHeapWellFormedAcceptor::accept(GCPointerBase &ptr) {
  accept(ptr.get(gc.getPointerBase()));
}
void CheckHeapWellFormedAcceptor::accept(GCHermesValueBase &hv) {
  acceptHV(hv);
}
void CheckHeapWellFormedAcceptor::accept(GCSmallHermesValueBase &hv) {
  acceptSHV(hv);
}
void CheckHeapWellFormedAcceptor::accept(const GCSymbolID &sym) {
  acceptSym(sym);
}

void CheckHeapWellFormedAcceptor::accept(const GCCell *ptr) {
  assert(
      (!ptr || gc.validPointer(ptr)) &&
      "A pointer is pointing outside of the valid region");
}

void CheckHeapWellFormedAcceptor::acceptWeakSym(WeakRootSymbolID &ws) {
  acceptSym(ws.getNoBarrierUnsafe());
}

void CheckHeapWellFormedAcceptor::acceptHV(HermesValue &hv) {
  assert(!hv.isInvalid() && "HermesValue with InvalidTag encountered by GC.");
  if (hv.isPointer()) {
    GCCell *cell = static_cast<GCCell *>(hv.getPointer());
    accept(cell);
  } else if (hv.isSymbol()) {
    acceptSym(hv.getSymbol());
  }
}

void CheckHeapWellFormedAcceptor::acceptSHV(SmallHermesValue &hv) {
  if (hv.isPointer()) {
    GCCell *cell = static_cast<GCCell *>(hv.getPointer(gc.getPointerBase()));
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
      gc.getCallbacks().isSymbolLive(sym) &&
      "Symbol is marked but is not live");
  // Check that the string used by this symbol is valid.
  accept(
      static_cast<const GCCell *>(gc.getCallbacks().getStringForSymbol(sym)));
}

#endif

} // namespace vm
} // namespace hermes
