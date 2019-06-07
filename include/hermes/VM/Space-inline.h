/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_SPACE_INLINE_H
#define HERMES_VM_SPACE_INLINE_H

#include "hermes/VM/GC.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/HermesValue-inline.h"
#include "hermes/VM/SlotAcceptorDefault.h"

namespace hermes {
namespace vm {

#ifndef HERMESVM_GC_NONCONTIG_GENERATIONAL

struct FullMSCUpdateAcceptor final : public SlotAcceptorDefault {
  static constexpr bool shouldMarkWeak = false;

  using SlotAcceptorDefault::accept;
  using SlotAcceptorDefault::SlotAcceptorDefault;
  void accept(void *&ptr) override {
    if (gc.contains(ptr)) {
      auto *cell = reinterpret_cast<GCCell *>(ptr);
      ptr = cell->getForwardingPointer();
    }
  }
  void accept(HermesValue &hv) override {
    if (hv.isPointer()) {
      auto *ptr = reinterpret_cast<GCCell *>(hv.getPointer());
      if (gc.contains(ptr)) {
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
    // Reset weak root if target GCCell is dead.
    ptr = gc.isMarked(cell) ? cell->getForwardingPointer() : nullptr;
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

#endif // HERMESVM_GC_NONCONTIG_GENERATIONAL

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_SPACE_INLINE_H
