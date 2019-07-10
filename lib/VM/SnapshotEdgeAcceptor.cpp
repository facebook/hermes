/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/SnapshotEdgeAcceptor.h"

namespace hermes {
namespace vm {

void SnapshotEdgeAcceptor::accept(void *&ptr, const char *name) {
  if (ptr) {
    snap.addEdge(V8HeapSnapshot::Edge{
        V8HeapSnapshot::Edge::Named{},
        V8HeapSnapshot::Edge ::Type::Internal,
        static_cast<V8HeapSnapshot::Node::ID>(ptrToOffset(ptr)),
        stringToID(name)});
  }
}

void SnapshotEdgeAcceptor::accept(HermesValue &hv, const char *name) {
  if (hv.isPointer()) {
    auto ptr = hv.getPointer();
    accept(ptr, name);
  }
}

void SnapshotEdgeAcceptor::accept(SymbolID sym, const char *name) {
  // snap->addSymbolIdToCurrentObject(name, sym.unsafeGetRaw());
}

void SnapshotEdgeAcceptor::accept(uint64_t value, const char *name) {
  // snap->addInternalToCurrentObject(name, value);
}

} // namespace vm
} // namespace hermes
