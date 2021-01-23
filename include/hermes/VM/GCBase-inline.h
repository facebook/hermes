/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_GCBASE_INLINE_H
#define HERMES_VM_GCBASE_INLINE_H

#include "hermes/VM/GC.h"
#include "hermes/VM/GCBase.h"

namespace hermes {
namespace vm {

template <typename Acceptor>
void GCBase::markWeakRefsIfNecessary(
    GCCell *cell,
    const VTable *vt,
    Acceptor &acceptor) {
  markWeakRefsIfNecessary(
      cell, vt, acceptor, std::is_convertible<Acceptor &, WeakRefAcceptor &>{});
}

template <typename Acceptor>
inline void GCBase::markCell(GCCell *cell, Acceptor &acceptor) {
  markCell(cell, cell->getVT(), acceptor);
}

template <typename Acceptor>
inline void
GCBase::markCell(GCCell *cell, const VTable *vt, Acceptor &acceptor) {
  SlotVisitor<Acceptor> visitor(acceptor);
  markCell(visitor, cell, vt);
}

template <typename Acceptor>
inline void GCBase::markCell(
    SlotVisitor<Acceptor> &visitor,
    GCCell *cell,
    const VTable *vt) {
  visitor.visit(cell, metaTable_[static_cast<size_t>(vt->kind)]);
  markWeakRefsIfNecessary(cell, vt, visitor.acceptor_);
}

template <typename Acceptor>
inline void GCBase::markCellWithinRange(
    SlotVisitor<Acceptor> &visitor,
    GCCell *cell,
    const VTable *vt,
    const char *begin,
    const char *end) {
  visitor.visitWithinRange(
      cell, metaTable_[static_cast<size_t>(vt->kind)], begin, end);
  markWeakRefsIfNecessary(cell, vt, visitor.acceptor_);
}

template <typename Acceptor>
inline void GCBase::markCellWithNames(
    SlotVisitorWithNames<Acceptor> &visitor,
    GCCell *cell) {
  const VTable *vt = cell->getVT();
  visitor.visit(cell, metaTable_[static_cast<size_t>(vt->kind)]);
  markWeakRefsIfNecessary(cell, vt, visitor.acceptor_);
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCBASE_INLINE_H
