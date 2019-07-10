/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/SnapshotAcceptor.h"

namespace hermes {
namespace vm {

void SnapshotRootAcceptor::accept(void *&ptr, const char *name) {
  // Ignore name for roots, it has no meaning currently.
  if (ptr) {
    snap->addRoot(ptrToOffset(ptr));
  }
}

void SnapshotRootAcceptor::accept(HermesValue &hv, const char *name) {
  if (hv.isPointer()) {
    auto *ptr = hv.getPointer();
    // Since this acceptor does not modify pointers, we don't need to worry
    // about writing the pointer value back into the hermes value.
    accept(ptr, name);
  }
}

void SnapshotAcceptor::accept(void *&ptr, const char *name) {
  if (ptr) {
    snap->addToCurrentObject(name, ptrToOffset(ptr));
  }
}

void SnapshotAcceptor::accept(HermesValue &hv, const char *name) {
  if (hv.isPointer()) {
    void *ptr = hv.getPointer();
    // Since accept does not modify its argument, no need to write back into
    // the hermes value.
    accept(ptr, name);
  } else {
    snap->addHermesValueToCurrentObject(name, hv);
  }
}

void SnapshotAcceptor::accept(SymbolID sym, const char *name) {
  snap->addSymbolIdToCurrentObject(name, sym.unsafeGetRaw());
}

void SnapshotAcceptor::accept(uint64_t value, const char *name) {
  snap->addInternalToCurrentObject(name, value);
}

} // namespace vm
} // namespace hermes
