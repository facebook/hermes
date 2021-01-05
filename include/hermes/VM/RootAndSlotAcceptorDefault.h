/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_ROOTANDSLOTACCEPTORDEFAULT_H
#define HERMES_VM_ROOTANDSLOTACCEPTORDEFAULT_H

#include "hermes/VM/GC.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/PointerBase.h"
#include "hermes/VM/SlotAcceptor.h"
#include "hermes/VM/WeakRef.h"

namespace hermes {
namespace vm {

/// A RootAndSlotAcceptorDefault provides a convenient overload for GCPointers
/// to be lowered into raw pointers.
struct RootAndSlotAcceptorDefault : public RootAndSlotAcceptor {
  GC &gc;
  RootAndSlotAcceptorDefault(GC &gc) : gc(gc) {}

  using RootAndSlotAcceptor::accept;

  virtual void accept(BasedPointer &ptr);

  void accept(GCPointerBase &ptr) override final {
    accept(ptr.getLoc(&gc));
  }

  void accept(PinnedHermesValue &hv) override final {
    acceptHV(hv);
  }

  void accept(GCHermesValue &hv) override final {
    acceptHV(hv);
  }

  virtual void acceptHV(HermesValue &hv) = 0;

  void accept(GCSymbolID sym) override {
    acceptSym(sym);
  }

  void accept(PinnedSymbolID sym) override {
    acceptSym(sym);
  }

  virtual void acceptSym(SymbolID sym) {
    // By default, symbol processing is a noop. Only a handful of acceptors
    // need to override it.
    // Not final because sometimes custom behavior is desired.
  }
};

/// A RootAndSlotAcceptorWithNamesDefault is similar to a
/// RootAndSlotAcceptorDefault, except it provides an overload for the named
/// acceptor of GCPointers.
struct RootAndSlotAcceptorWithNamesDefault
    : public RootAndSlotAcceptorWithNames {
  GC &gc;
  RootAndSlotAcceptorWithNamesDefault(GC &gc) : gc(gc) {}

  using RootAndSlotAcceptorWithNames::accept;

  void accept(BasedPointer &ptr, const char *name) {
    // See comments in RootAndSlotAcceptorDefault::accept(BasedPointer &) for
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

  void accept(PinnedHermesValue &hv, const char *name) override {
    acceptHV(hv, name);
  }

  void accept(GCHermesValue &hv, const char *name) override {
    acceptHV(hv, name);
  }

  virtual void acceptHV(HermesValue &hv, const char *name) = 0;

  void accept(PinnedSymbolID sym, const char *name) override {
    acceptSym(sym, name);
  }

  void accept(GCSymbolID sym, const char *name) override {
    acceptSym(sym, name);
  }

  virtual void acceptSym(SymbolID sym, const char *name) {
    // By default, symbol processing is a noop. Only a handful of acceptors
    // need to override it.
    // Not final because sometimes custom behavior is desired.
  }
};

struct WeakRootAcceptorDefault : public WeakRootAcceptor {
  GC &gcForWeakRootDefault;

  WeakRootAcceptorDefault(GC &gc) : gcForWeakRootDefault(gc) {}

  void acceptWeak(WeakRootBase &ptr) override final;

  /// Subclasses override this implementation instead of accept(WeakRootBase &).
  virtual void acceptWeak(void *&ptr) = 0;

  /// This gets a default implementation: extract the real pointer to a local,
  /// call acceptWeak on that, write the result back as a BasedPointer.
  inline virtual void acceptWeak(BasedPointer &ptr);
};

/// @name Inline implementations.
/// @{

inline void WeakRootAcceptorDefault::acceptWeak(WeakRootBase &ptr) {
  GCPointerBase::StorageType weakRootStorage = ptr.getNoBarrierUnsafe();
  acceptWeak(weakRootStorage);
  // Assign back to the input pointer location.
  ptr = weakRootStorage;
}

inline void RootAndSlotAcceptorDefault::accept(BasedPointer &ptr) {
  if (!ptr) {
    return;
  }
  // accept takes an l-value reference and potentially writes to it.
  // Write the value back out to the BasedPointer.
  PointerBase *const base = gc.getPointerBase();
  void *actualizedPointer = base->basedToPointerNonNull(ptr);
  accept(actualizedPointer);
  // Assign back to the based pointer.
  ptr = base->pointerToBased(actualizedPointer);
}

inline void WeakRootAcceptorDefault::acceptWeak(BasedPointer &ptr) {
  if (!ptr) {
    return;
  }
  // accept takes an l-value reference and potentially writes to it.
  // Write the value back out to the BasedPointer.
  PointerBase *const base = gcForWeakRootDefault.getPointerBase();
  void *actualizedPointer = base->basedToPointerNonNull(ptr);
  acceptWeak(actualizedPointer);
  // Assign back to the based pointer.
  ptr = base->pointerToBased(actualizedPointer);
}

/// @}

} // namespace vm
} // namespace hermes

#endif
