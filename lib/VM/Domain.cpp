/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/Domain.h"

namespace hermes {
namespace vm {

VTable Domain::vt{CellKind::DomainKind,
                  sizeof(Domain),
                  _finalizeImpl,
                  _markWeakImpl,
                  _mallocSizeImpl};

void DomainBuildMeta(const GCCell *cell, Metadata::Builder &mb) {}

PseudoHandle<Domain> Domain::create(Runtime *runtime) {
  void *mem =
      runtime->alloc</*fixedSize*/ true, HasFinalizer::Yes>(sizeof(Domain));
  auto self = createPseudoHandle(new (mem) Domain(runtime));
  return self;
}

void Domain::_finalizeImpl(GCCell *cell, GC *gc) {
  auto *self = vmcast<Domain>(cell);
  self->~Domain();
}

Domain::~Domain() {
  for (RuntimeModule *rm : runtimeModules_) {
    delete rm;
  }
}

void Domain::_markWeakImpl(GCCell *cell, GC *gc) {
  auto *self = vmcast_during_gc<Domain>(cell, gc);
  self->markWeakRefs(gc);
}

void Domain::markWeakRefs(GC *gc) {
  for (RuntimeModule *rm : runtimeModules_) {
    rm->markDomainRef(gc);
  }
}

size_t Domain::_mallocSizeImpl(GCCell *cell) {
  auto *self = vmcast<Domain>(cell);
  return self->runtimeModules_.capacity_in_bytes();
}

} // namespace vm
} // namespace hermes
