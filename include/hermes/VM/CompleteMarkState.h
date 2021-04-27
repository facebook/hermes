/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_COMPLETEMARKSTATE_H
#define HERMES_VM_COMPLETEMARKSTATE_H

#include "hermes/VM/GCBase.h"
#include "hermes/VM/GCCell.h"
#include "hermes/VM/GCDecl.h"
#include "hermes/VM/MarkBitArrayNC.h"

#include <list>
#include <vector>

namespace hermes {
namespace vm {

/// Forward declartions.
struct FullMSCUpdateAcceptor;

template <CellKind>
class JSWeakMapImpl;
using JSWeakMap = JSWeakMapImpl<CellKind::WeakMapKind>;

/// Intermediate state from marking.
struct CompleteMarkState {
  /// Forward declaration of the Acceptor used to mark fields of marked objects.
  struct FullMSCMarkTransitiveAcceptor;

  /// The mark behavior used during completeMarking:
  /// \pre The object at ptr has been found to be reachable.
  /// \post Its mark bit is set.  If it had been unmarked before, the
  /// object is made gray (i.e., by pushing it on the mark stack if ptr
  /// is not guaranteed to be visited on the current mark bit array
  /// traversal).
  void markTransitive(void *ptr);

  /// The pointer \p cell is assumed to point to a reachable object.
  /// Push it on the appropriate markStack.
  void pushCell(GCCell *cell);

  /// Continually pops elements from the mark stack and scans their pointer
  /// fields.  If such a field points to an unmarked object, mark it and push it
  /// on the mark stack.  If such a push would overflow the mark stack, sets a
  /// flag on this CompleteMarkState to indicate this.  Returns if this overflow
  /// happens, or when the mark stack is empty.
  void drainMarkStack(GenGC *gc, FullMSCMarkTransitiveAcceptor &acceptor);

  /// The maximum size of the mark stack.
  static const size_t kMarkStackLimit = 1000;

  /// Maximum number of pointers that can be pushed onto the mark stacks by
  /// one variable sized object. If the max is reached, later mark calls do
  /// not do anything. This will require the object to be scanned again, and
  /// again until all of its fields are marked. Since we must loop through
  /// each object's fields in the current implementation of vtp->mark, we do
  /// not want this variable set too low, since we could loop through all the
  /// fields many times. However, setting this variable too high will not reap
  /// the benefits of reducing the potential size of the mark stacks.

  /// TODO: There is an upcoming change that will allow for more efficient
  /// iteration through variable sized cells, starting at a specified index.
  /// This will allow us to lower this limit without hurting performance.
  static const uint32_t kMaxPtrsPushedByVarParent = 200;

  // TODO: Test whether it is better to use llvh::SmallVector for the mark
  // stacks.

  /// The mark stack for fixed-sized objects that have been marked but not yet
  /// scanned.
  std::vector<GCCell *> markStack_;

  /// The mark stack for variable sized objects that have been marked but not
  /// yet fully scanned.
  std::vector<GCCell *> varSizeMarkStack_;

  /// Check if the current object going through complete marking is var sized.
  bool markingVarSizeCell = false;

  /// The current number of pointers that have been pushed by the same object.
  /// Used to check if the number pushed has passed the maximum for var sized
  /// cells.
  uint32_t numPtrsPushedByParent = 0;

  /// Whether one of the mark stacks has overflowed since the current
  /// mark-bit-map traversal was started.
  bool markStackOverflow_ = false;

  /// Whether to do "proper" weak map marking. Under control of this boolean
  /// because there have been bugs, so we want, at least for a while,
  /// to be able to turn this off with a GK, reverting to the
  /// previous, non-spec-compliant, behavior.
  bool properWeakMapMarking_;

  /// The total number of mark stack overflows that have occurred.
  unsigned numMarkStackOverflows_{0};

  /// Stores the current object whose fields are being scanned
  /// to be marked if needed.
  GCCell *currentParPointer = nullptr;

  /// The WeakMap objects that have been discovered to be reachable.
  std::vector<JSWeakMap *> reachableWeakMaps_;
};

/// Returns a heap acceptor for mark-sweep-compact pointer update.
std::unique_ptr<FullMSCUpdateAcceptor> getFullMSCUpdateAcceptor(GenGC &gc);

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_COMPLETEMARKSTATE_H
