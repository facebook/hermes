/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_DOMAIN_H
#define HERMES_VM_DOMAIN_H

#include "hermes/VM/CopyableVector.h"
#include "hermes/VM/GCCell.h"
#include "hermes/VM/Runtime.h"

namespace hermes {
namespace vm {

/// A Domain is a GC-managed proxy for a set of RuntimeModules which were
/// compiled at the same time, and thus are intended to work together.
/// For example, they can provide a set of CommonJS modules.
///
/// A Domain is constructed empty, and then populated with RuntimeModules,
/// which it then keeps alive for its entire lifetime. All JSFunctions which are
/// constructed in the given Domain then keep strong GC pointers to it.
///
/// When a Domain is garbage collected, all functions that can run in the domain
/// must be unreachable. We can therefore free all RuntimeModules within the
/// domain, knowing there are no other ways to run any code dependent on the
/// Domain.
///
/// Each RuntimeModule keeps a weak reference to its owning Domain,
/// and those weak references are marked when the Domain marks its own WeakRefs.
class Domain final : public GCCell {
  using Super = GCCell;

  static VTable vt;

  /// RuntimeModules owned by this Domain.
  /// These will be freed from the Domain destructor.
  CopyableVector<RuntimeModule *> runtimeModules_{};

 public:
  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::DomainKind;
  }

  /// Create a Domain with no associated RuntimeModules.
  static PseudoHandle<Domain> create(Runtime *runtime);

  /// Add \p runtimeModule to the list of RuntimeModules owned by this domain.
  static void addRuntimeModule(
      Handle<Domain> self,
      Runtime *runtime,
      RuntimeModule *runtimeModule) {
    self->runtimeModules_.push_back(runtimeModule, &runtime->getHeap());
  }

 private:
  /// Create a domain with no associated RuntimeModules.
  Domain(Runtime *runtime) : GCCell(&runtime->getHeap(), &vt) {}

  /// Destroy associated RuntimeModules.
  ~Domain();

  /// Free all non-GC managed resources associated with the object.
  static void _finalizeImpl(GCCell *cell, GC *gc);

  /// Mark all the weak references for an object.
  static void _markWeakImpl(GCCell *cell, GC *gc);

  /// \return the amount of non-GC memory being used by the given \p cell.
  static size_t _mallocSizeImpl(GCCell *cell);

  /// Mark the WeakRefs in associated RuntimeModules which point to this Domain.
  void markWeakRefs(GC *gc);
};

} // namespace vm
} // namespace hermes

#endif
