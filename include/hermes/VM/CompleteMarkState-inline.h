/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_COMPLETEMARKSTATE_INLINE_H
#define HERMES_VM_COMPLETEMARKSTATE_INLINE_H

#include "hermes/VM/CompleteMarkState.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/HermesValue-inline.h"
#include "hermes/VM/RootAndSlotAcceptorDefault.h"

namespace hermes {
namespace vm {

struct CompleteMarkState::FullMSCMarkTransitiveAcceptor final
    : public RootAndSlotAcceptorDefault {
  CompleteMarkState *markState;
  FullMSCMarkTransitiveAcceptor(GC &gc, CompleteMarkState *markState)
      : RootAndSlotAcceptorDefault(gc), markState(markState) {}

  using RootAndSlotAcceptorDefault::accept;

  void accept(void *&ptr) override {
    if (ptr) {
      assert(gc.dbgContains(ptr));
      markState->markTransitive(ptr);
    }
  }
  void acceptHV(HermesValue &hv) override {
    if (hv.isPointer()) {
      void *cell = hv.getPointer();
      accept(cell);
    } else if (hv.isSymbol()) {
      acceptSym(hv.getSymbol());
    }
  }
  void acceptSym(SymbolID sym) override {
    gc.markSymbol(sym);
  }
};

/// This acceptor is used for updating pointers via forwarding pointers
/// in mark/sweep/compact.
struct FullMSCUpdateAcceptor final : public RootAndSlotAcceptorDefault,
                                     public WeakRootAcceptorDefault {
  using RootAndSlotAcceptorDefault::accept;
  using WeakRootAcceptorDefault::acceptWeak;

  FullMSCUpdateAcceptor(GC &gc)
      : RootAndSlotAcceptorDefault(gc), WeakRootAcceptorDefault(gc) {}

  void accept(void *&ptr) override {
    if (ptr) {
      assert(gc.dbgContains(ptr) && "ptr not in heap");
      auto *cell = reinterpret_cast<GCCell *>(ptr);
      ptr = cell->getForwardingPointer();
    }
  }

  void acceptWeak(void *&ptr) override {
    if (ptr == nullptr) {
      return;
    }
    auto *cell = reinterpret_cast<GCCell *>(ptr);
    assert(gc.dbgContains(ptr) && "ptr not in heap");
    // Reset weak root if target GCCell is dead.
    ptr = AlignedHeapSegment::getCellMarkBit(cell)
        ? cell->getForwardingPointer()
        : nullptr;
  }

  void acceptHV(HermesValue &hv) override {
    if (hv.isPointer()) {
      auto *ptr = reinterpret_cast<GCCell *>(hv.getPointer());
      if (ptr) {
        assert(gc.dbgContains(ptr) && "ptr not in heap");
        hv.setInGC(hv.updatePointer(ptr->getForwardingPointer()), &gc);
      }
    }
  }

  void accept(WeakRefBase &wr) override {
    // This acceptor is used once it is known where all live data is, so now is
    // the time to mark whether a weak ref is known.
    WeakRefSlot *slot = wr.unsafeGetSlot();
    assert(
        slot->state() != WeakSlotState::Free &&
        "marking a freed weak ref slot");
    slot->mark();
  }
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_COMPLETEMARKSTATE_INLINE_H
