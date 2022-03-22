/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_ROOTANDSLOTACCEPTORDEFAULT_H
#define HERMES_VM_ROOTANDSLOTACCEPTORDEFAULT_H

#include "hermes/VM/PointerBase.h"
#include "hermes/VM/SlotAcceptor.h"
#include "hermes/VM/WeakRef.h"
#include "hermes/VM/WeakRoot.h"

namespace hermes {
namespace vm {

/// A RootAndSlotAcceptorDefault provides a convenient overload for GCPointers
/// to be lowered into raw pointers.
class RootAndSlotAcceptorDefault : public RootAndSlotAcceptor {
 public:
  explicit RootAndSlotAcceptorDefault(PointerBase &pointerBase)
      : pointerBase_(pointerBase) {}

  using RootAndSlotAcceptor::accept;

  void accept(GCPointerBase &ptr) final {
    auto *p = ptr.get(pointerBase_);
    accept(p);
    ptr.setInGC(CompressedPointer::encode(p, pointerBase_));
  }

  void accept(PinnedHermesValue &hv) final {
    assert((!hv.isPointer() || hv.getPointer()) && "Value is not nullable.");
    acceptHV(hv);
  }
  void acceptNullable(PinnedHermesValue &hv) final {
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
  PointerBase &pointerBase_;
};

/// A RootAndSlotAcceptorWithNamesDefault is similar to a
/// RootAndSlotAcceptorDefault, except it provides an overload for the named
/// acceptor of GCPointers.
class RootAndSlotAcceptorWithNamesDefault
    : public RootAndSlotAcceptorWithNames {
 public:
  explicit RootAndSlotAcceptorWithNamesDefault(PointerBase &pointerBase)
      : pointerBase_(pointerBase) {}

  using RootAndSlotAcceptorWithNames::accept;

  void accept(GCPointerBase &ptr, const char *name) final {
    auto *p = ptr.get(pointerBase_);
    accept(p, name);
    ptr.setInGC(CompressedPointer::encode(p, pointerBase_));
  }

  void accept(PinnedHermesValue &hv, const char *name) final {
    assert((!hv.isPointer() || hv.getPointer()) && "Value is not nullable.");
    acceptHV(hv, name);
  }
  void acceptNullable(PinnedHermesValue &hv, const char *name) final {
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
  PointerBase &pointerBase_;
};

class WeakAcceptorDefault : public WeakRefAcceptor, public WeakRootAcceptor {
 public:
  explicit WeakAcceptorDefault(PointerBase &base)
      : pointerBaseForWeakRoot_(base) {}

  void acceptWeak(WeakRootBase &ptr) final;

  /// Subclasses override this implementation instead of accept(WeakRootBase &).
  virtual void acceptWeak(GCCell *&ptr) = 0;

 protected:
  // Named differently to avoid collisions with
  // RootAndSlotAcceptorDefault::pointerBase_.
  PointerBase &pointerBaseForWeakRoot_;
};

/// @name Inline implementations.
/// @{

inline void WeakAcceptorDefault::acceptWeak(WeakRootBase &ptr) {
  GCCell *p = ptr.getNoBarrierUnsafe(pointerBaseForWeakRoot_);
  acceptWeak(p);
  ptr = CompressedPointer::encode(p, pointerBaseForWeakRoot_);
}

/// @}

} // namespace vm
} // namespace hermes

#endif
