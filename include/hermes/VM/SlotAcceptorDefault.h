/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_SLOTACCEPTORDEFAULT_H
#define HERMES_VM_SLOTACCEPTORDEFAULT_H

#include "hermes/VM/GC.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/SlotAcceptor.h"

namespace hermes {
namespace vm {

/// A SlotAcceptorDefault provides a convenient overload for GCPointers to be
/// lowered into raw pointers.
struct SlotAcceptorDefault : public SlotAcceptor {
  GC &gc;
  SlotAcceptorDefault(GC &gc) : gc(gc) {}

  using SlotAcceptor::accept;

  void accept(BasedPointer &ptr) override {
    if (!ptr) {
      return;
    }
    // accept takes an l-value reference and potentially writes to it.
    // Write the value back out to the BasedPointer.
    PointerBase *const base = gc.getPointerBase();
    void *actualizedPointer = base->basedToPointerNonNull(ptr);
    accept(actualizedPointer);
    // Assign back to the based pointer.
    ptr = base->pointerToBasedNonNull(actualizedPointer);
  }

  void accept(GCPointerBase &ptr) override final {
    accept(ptr.getLoc(&gc));
  }
};

/// A SlotAcceptorWithNamesDefault is similar to a SlotAcceptorDefault, except
/// it provides an overload for the named acceptor of GCPointers.
struct SlotAcceptorWithNamesDefault : public RootAcceptor {
  GC &gc;
  SlotAcceptorWithNamesDefault(GC &gc) : gc(gc) {}

  using SlotAcceptorWithNames::accept;

  void accept(BasedPointer &ptr, const char *name) override {
    // See comments in SlotAcceptorDefault::accept(BasedPointer &) for
    // explanation.
    if (!ptr) {
      return;
    }
    PointerBase *const base = gc.getPointerBase();
    void *actualizedPointer = base->basedToPointerNonNull(ptr);
    accept(actualizedPointer, name);
    ptr = base->pointerToBasedNonNull(actualizedPointer);
  }

  void accept(GCPointerBase &ptr, const char *name) override final {
    accept(ptr.getLoc(&gc), name);
  }
};

} // namespace vm
} // namespace hermes

#endif
