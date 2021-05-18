/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_ROOTANDSLOTACCEPTORDEFAULT_H
#define HERMES_VM_ROOTANDSLOTACCEPTORDEFAULT_H

#include "hermes/VM/PointerBase.h"
#include "hermes/VM/SlotAcceptor.h"
#include "hermes/VM/WeakRef.h"

namespace hermes {
namespace vm {

/// A RootAndSlotAcceptorDefault provides a convenient overload for GCPointers
/// to be lowered into raw pointers.
class RootAndSlotAcceptorDefault : public RootAndSlotAcceptor {
 public:
  explicit RootAndSlotAcceptorDefault(PointerBase *pointerBase)
      : pointerBase_(pointerBase) {}

  using RootAndSlotAcceptor::accept;

  virtual void accept(BasedPointer &ptr);

  void accept(GCPointerBase &ptr) final {
    accept(ptr.getLoc());
  }

  void accept(PinnedHermesValue &hv) final {
    acceptHV(hv);
  }

  void accept(GCHermesValue &hv) final {
    acceptHV(hv);
  }

  virtual void acceptHV(HermesValue &hv) = 0;

  void accept(GCSmallHermesValue &shv) final {
    acceptSHV(shv);
  }

  virtual void acceptSHV(SmallHermesValue &hv) = 0;

  void accept(const GCSymbolID &sym) final {
    acceptSym(sym);
  }

  void accept(const RootSymbolID &sym) final {
    acceptSym(sym);
  }

  virtual void acceptSym(SymbolID sym) {
    // By default, symbol processing is a noop. Only a handful of acceptors
    // need to override it.
    // Not final because sometimes custom behavior is desired.
  }

 protected:
  PointerBase *pointerBase_;
};

/// A RootAndSlotAcceptorWithNamesDefault is similar to a
/// RootAndSlotAcceptorDefault, except it provides an overload for the named
/// acceptor of GCPointers.
class RootAndSlotAcceptorWithNamesDefault
    : public RootAndSlotAcceptorWithNames {
 public:
  explicit RootAndSlotAcceptorWithNamesDefault(PointerBase *pointerBase)
      : pointerBase_(pointerBase) {}

  using RootAndSlotAcceptorWithNames::accept;

  void accept(BasedPointer &ptr, const char *name) {
    // See comments in RootAndSlotAcceptorDefault::accept(BasedPointer &) for
    // explanation.
    if (!ptr) {
      return;
    }
    GCCell *actualizedPointer =
        static_cast<GCCell *>(pointerBase_->basedToPointerNonNull(ptr));
    accept(actualizedPointer, name);
    ptr = pointerBase_->pointerToBasedNonNull(actualizedPointer);
  }

  void accept(GCPointerBase &ptr, const char *name) final {
    accept(ptr.getLoc(), name);
  }

  void accept(PinnedHermesValue &hv, const char *name) final {
    acceptHV(hv, name);
  }

  void accept(GCHermesValue &hv, const char *name) final {
    acceptHV(hv, name);
  }

  virtual void acceptHV(HermesValue &hv, const char *name) = 0;

  void accept(GCSmallHermesValue &shv, const char *name) final {
    acceptSHV(shv, name);
  }

  virtual void acceptSHV(SmallHermesValue &hv, const char *name) = 0;

  void accept(const RootSymbolID &sym, const char *name) final {
    acceptSym(sym, name);
  }

  void accept(const GCSymbolID &sym, const char *name) final {
    acceptSym(sym, name);
  }

  virtual void acceptSym(SymbolID sym, const char *name) {
    // By default, symbol processing is a noop. Only a handful of acceptors
    // need to override it.
    // Not final because sometimes custom behavior is desired.
  }

 protected:
  PointerBase *pointerBase_;
};

class WeakAcceptorDefault : public WeakRefAcceptor, public WeakRootAcceptor {
 public:
  explicit WeakAcceptorDefault(PointerBase *base)
      : pointerBaseForWeakRoot_(base) {}

  void acceptWeak(WeakRootBase &ptr) final;

  /// Subclasses override this implementation instead of accept(WeakRootBase &).
  virtual void acceptWeak(GCCell *&ptr) = 0;

  /// This gets a default implementation: extract the real pointer to a local,
  /// call acceptWeak on that, write the result back as a BasedPointer.
  inline virtual void acceptWeak(BasedPointer &ptr);

 protected:
  // Named differently to avoid collisions with
  // RootAndSlotAcceptorDefault::pointerBase_.
  PointerBase *pointerBaseForWeakRoot_;
};

/// @name Inline implementations.
/// @{

inline void RootAndSlotAcceptorDefault::accept(BasedPointer &ptr) {
  if (!ptr) {
    return;
  }
  // accept takes an l-value reference and potentially writes to it.
  // Write the value back out to the BasedPointer.
  GCCell *actualizedPointer =
      static_cast<GCCell *>(pointerBase_->basedToPointerNonNull(ptr));
  accept(actualizedPointer);
  // Assign back to the based pointer.
  ptr = pointerBase_->pointerToBased(actualizedPointer);
}

inline void WeakAcceptorDefault::acceptWeak(WeakRootBase &ptr) {
  acceptWeak(ptr.getLocNoBarrierUnsafe());
}

inline void WeakAcceptorDefault::acceptWeak(BasedPointer &ptr) {
  if (!ptr) {
    return;
  }
  // accept takes an l-value reference and potentially writes to it.
  // Write the value back out to the BasedPointer.
  GCCell *actualizedPointer = static_cast<GCCell *>(
      pointerBaseForWeakRoot_->basedToPointerNonNull(ptr));
  acceptWeak(actualizedPointer);
  // Assign back to the based pointer.
  ptr = pointerBaseForWeakRoot_->pointerToBased(actualizedPointer);
}

/// @}

} // namespace vm
} // namespace hermes

#endif
