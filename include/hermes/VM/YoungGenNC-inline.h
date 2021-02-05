/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_YOUNGGENNC_INL_H
#define HERMES_VM_YOUNGGENNC_INL_H

#include "hermes/VM/YoungGenNC.h"

#include "hermes/VM/GC.h"
#include "hermes/VM/RootAndSlotAcceptorDefault.h"

namespace hermes {
namespace vm {

/// The acceptor used to evacuate the young generation.  For each
/// pointer-containing slot, ensure that if it points to a young-gen
/// object, that object is evacuated, and the slot updated.
struct YoungGen::EvacAcceptor final : public RootAndSlotAcceptorDefault {
  YoungGen &gen;
  EvacAcceptor(GC &gc, YoungGen &gen)
      : RootAndSlotAcceptorDefault(gc.getPointerBase()), gen(gen) {}

  using RootAndSlotAcceptorDefault::accept;

  void accept(BasedPointer &basedPtr) override {
    gen.ensureReferentCopied(&basedPtr);
  }

  void accept(GCCell *&ptr) override {
    gen.ensureReferentCopied(&ptr);
  }

  void acceptHV(HermesValue &hv) override {
    if (hv.isPointer()) {
      gen.ensureReferentCopied(&hv);
    }
  }
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_YOUNGGENNC_INL_H
