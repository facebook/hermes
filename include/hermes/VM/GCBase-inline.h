/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_GCBASE_INLINE_H
#define HERMES_VM_GCBASE_INLINE_H

#include "hermes/VM/GC.h"
#include "hermes/VM/GCBase.h"

namespace hermes {
namespace vm {

template <typename Acceptor>
void GCBase::markWeakRefsIfNecessary(
    GC *gc,
    GCCell *cell,
    const VTable *vt,
    Acceptor &acceptor) {
  markWeakRefsIfNecessary(
      gc,
      cell,
      vt,
      acceptor,
      std::is_convertible<Acceptor &, WeakRefAcceptor &>{});
}

template <typename Acceptor>
inline void GCBase::markCell(GCCell *cell, GC *gc, Acceptor &acceptor) {
  markCell(cell, cell->getVT(), gc, acceptor);
}

template <typename Acceptor>
inline void
GCBase::markCell(GCCell *cell, const VTable *vt, GC *gc, Acceptor &acceptor) {
  SlotVisitor<Acceptor> visitor(acceptor);
  markCell(visitor, cell, vt, gc);
}

template <typename Acceptor>
inline void GCBase::markCell(
    SlotVisitor<Acceptor> &visitor,
    GCCell *cell,
    const VTable *vt,
    GC *gc) {
  visitor.visit(cell, gc->metaTable_[static_cast<size_t>(vt->kind)]);
  markWeakRefsIfNecessary(gc, cell, vt, visitor.acceptor_);
}

template <typename Acceptor>
inline void GCBase::markCellWithinRange(
    SlotVisitor<Acceptor> &visitor,
    GCCell *cell,
    const VTable *vt,
    GC *gc,
    const char *begin,
    const char *end) {
  visitor.visitWithinRange(
      cell, gc->metaTable_[static_cast<size_t>(vt->kind)], begin, end);
  markWeakRefsIfNecessary(gc, cell, vt, visitor.acceptor_);
}

template <typename Acceptor>
inline void GCBase::markCellWithNames(
    SlotVisitorWithNames<Acceptor> &visitor,
    GCCell *cell,
    GC *gc) {
  const VTable *vt = cell->getVT();
  visitor.visit(cell, gc->metaTable_[static_cast<size_t>(vt->kind)]);
  markWeakRefsIfNecessary(gc, cell, vt, visitor.acceptor_);
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCBASE_INLINE_H
