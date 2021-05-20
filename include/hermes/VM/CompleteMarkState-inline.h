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
#include "hermes/VM/SmallHermesValue-inline.h"

namespace hermes {
namespace vm {

struct CompleteMarkState::FullMSCMarkTransitiveAcceptor final
    : public RootAndSlotAcceptorDefault {
  GenGC &gc;
  CompleteMarkState *markState;
  FullMSCMarkTransitiveAcceptor(GenGC &gc, CompleteMarkState *markState)
      : RootAndSlotAcceptorDefault(gc.getPointerBase()),
        gc(gc),
        markState(markState) {}

  using RootAndSlotAcceptorDefault::accept;

  void accept(GCCell *&ptr) override {
    if (ptr) {
      assert(gc.dbgContains(ptr));
      markState->markTransitive(ptr);
    }
  }
  void acceptHV(HermesValue &hv) override {
    if (hv.isPointer()) {
      GCCell *cell = static_cast<GCCell *>(hv.getPointer());
      accept(cell);
    } else if (hv.isSymbol()) {
      acceptSym(hv.getSymbol());
    }
  }
  void acceptSHV(SmallHermesValue &hv) override {
    if (hv.isPointer()) {
      GCCell *cell = static_cast<GCCell *>(hv.getPointer(pointerBase_));
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
                                     public WeakAcceptorDefault {
  GenGC &gc;
  using RootAndSlotAcceptorDefault::accept;
  using WeakAcceptorDefault::acceptWeak;

  FullMSCUpdateAcceptor(GenGC &gc)
      : RootAndSlotAcceptorDefault(gc.getPointerBase()),
        WeakAcceptorDefault(gc.getPointerBase()),
        gc(gc) {}

  void accept(GCCell *&ptr) override {
    if (ptr) {
      assert(gc.dbgContains(ptr) && "ptr not in heap");
      ptr = ptr->getForwardingPointer().getNonNull(pointerBase_);
    }
  }

  void acceptWeak(GCCell *&ptr) override {
    if (ptr == nullptr) {
      return;
    }
    assert(gc.dbgContains(ptr) && "ptr not in heap");
    // Reset weak root if target GCCell is dead.
    ptr = AlignedHeapSegment::getCellMarkBit(ptr)
        ? ptr->getForwardingPointer().getNonNull(pointerBase_)
        : nullptr;
  }

  void acceptHV(HermesValue &hv) override {
    if (hv.isPointer()) {
      auto *ptr = static_cast<GCCell *>(hv.getPointer());
      if (ptr) {
        assert(gc.dbgContains(ptr) && "ptr not in heap");
        hv.setInGC(
            hv.updatePointer(
                ptr->getForwardingPointer().getNonNull(pointerBase_)),
            &gc);
      }
    }
  }

  void acceptSHV(SmallHermesValue &hv) override {
    if (hv.isPointer()) {
      auto *ptr = static_cast<GCCell *>(hv.getPointer(pointerBase_));
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
