/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_SKIPWEAKACCEPTOR_H
#define HERMES_VM_SKIPWEAKACCEPTOR_H

#include "hermes/VM/GC.h"
#include "hermes/VM/RootAndSlotAcceptorDefault.h"

namespace hermes {
namespace vm {

/// In WeakMap marking, if we mark the weak ref slots referenced by a
/// WeakMap early, this breaks assertions when we attempt to access
/// the slots while determining which keys are reachable.  This
/// Acceptor type delegates to another Acceptor, except when marking
/// WeakRefs -- those are ignored.  The type of the "target" acceptor, to which
/// the calls are delegated, is a template parameter so that those calls are
/// non-virtual (since we know the exact type).
template <typename TargetAcceptor>
class SkipWeakRefsAcceptor final : public SlotAcceptor {
 public:
  SkipWeakRefsAcceptor(GC &gc, TargetAcceptor *acceptor)
      : acceptor_(acceptor) {}
  void accept(GCPointerBase &ptr) override {
    acceptor_->accept(ptr);
  }
  void accept(GCHermesValue &hv) override {
    acceptor_->accept(hv);
  }
  void accept(GCSmallHermesValue &hv) override {
    acceptor_->accept(hv);
  }
  void accept(const GCSymbolID &sym) override {
    acceptor_->accept(sym);
  }

 private:
  TargetAcceptor *acceptor_;
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_SKIPWEAKACCEPTOR_H
