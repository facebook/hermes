/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_COMPLETEMARKSTATE_INLINE_H
#define HERMES_VM_COMPLETEMARKSTATE_INLINE_H

#include "hermes/VM/CompleteMarkState.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/HermesValue-inline.h"
#include "hermes/VM/SlotAcceptorDefault.h"

namespace hermes {
namespace vm {

struct CompleteMarkState::FullMSCMarkTransitiveAcceptor final
    : public SlotAcceptorDefault {
  CompleteMarkState *markState;
  FullMSCMarkTransitiveAcceptor(GC &gc, CompleteMarkState *markState)
      : SlotAcceptorDefault(gc), markState(markState) {}

  using SlotAcceptorDefault::accept;

  void accept(void *&ptr) override {
    if (ptr) {
      assert(gc.dbgContains(ptr));
      markState->markTransitive(ptr);
    }
  }
  void accept(HermesValue &hv) override {
    if (hv.isPointer()) {
      void *cell = hv.getPointer();
      accept(cell);
    } else if (hv.isSymbol()) {
      accept(hv.getSymbol());
    }
  }
  void accept(SymbolID sym) override {
    gc.markSymbol(sym);
  }
};

/// This acceptor is used for updating pointers via forwarding pointers
/// in mark/sweep/compact.
struct FullMSCUpdateAcceptor final : public SlotAcceptorDefault {
  static constexpr bool shouldMarkWeak = false;

  using SlotAcceptorDefault::accept;
  using SlotAcceptorDefault::SlotAcceptorDefault;
  void accept(void *&ptr) override {
    if (ptr) {
      assert(gc.dbgContains(ptr) && "ptr not in heap");
      auto *cell = reinterpret_cast<GCCell *>(ptr);
      ptr = cell->getForwardingPointer();
    }
  }
  void accept(HermesValue &hv) override {
    if (hv.isPointer()) {
      auto *ptr = reinterpret_cast<GCCell *>(hv.getPointer());
      if (ptr) {
        assert(gc.dbgContains(ptr) && "ptr not in heap");
        hv.setInGC(hv.updatePointer(ptr->getForwardingPointer()), &gc);
      }
    }
  }
};

/// This acceptor is used for updating weak roots pointers via
/// forwarding pointers in mark/sweep/compact.
/// If the target GCCell is still alive the weak roots are updated using
/// forwarding pointers; otherwise, they are reset to nullptr.
struct FullMSCUpdateWeakRootsAcceptor final : public SlotAcceptorDefault {
  static constexpr bool shouldMarkWeak = false;

  using SlotAcceptorDefault::accept;
  using SlotAcceptorDefault::SlotAcceptorDefault;
  void accept(void *&ptr) override {
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
  void accept(HermesValue &hv) override {
    if (!hv.isPointer()) {
      return;
    }
    void *ptr = hv.getPointer();
    accept(ptr);
    hv.setInGC(hv.updatePointer(ptr), &gc);
  }
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_COMPLETEMARKSTATE_INLINE_H
