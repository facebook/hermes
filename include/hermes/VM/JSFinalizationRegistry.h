/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "hermes/VM/Callable.h"
#include "hermes/VM/CellKind.h"
#include "hermes/VM/JSObject.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/WeakRef.h"

namespace hermes {
namespace vm {

/// A single registration cell in FinalizationRegistry.
class FinalizationRecord : public GCCell {
 public:
  static const VTable vt;

  static constexpr CellKind getCellKind() {
    return CellKind::FinalizationRecordKind;
  }

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::FinalizationRecordKind;
  }

  friend void FinalizationRecordBuildMeta(
      const GCCell *cell,
      Metadata::Builder &mb);

  /// Free this record and WeakRefSlots it holds.
  static void _finalizeImpl(GCCell *cell, GC &gc);

#ifdef HERMES_MEMORY_INSTRUMENTATION
  static void _snapshotAddEdgesImpl(GCCell *cell, GC &gc, HeapSnapshot &snap);
#endif

  static HermesValue create(
      Runtime &runtime,
      Handle<> target,
      Handle<> heldValue,
      Handle<> unregisterToken);

  FinalizationRecord(
      Runtime &runtime,
      const PinnedValue<SmallHermesValue> &targetPV,
      Handle<> heldValue,
      const PinnedValue<SmallHermesValue> &unregisterTokenPV)
      : target(runtime, targetPV.getSmallHermesValue()),
        heldValue(*heldValue, runtime.getHeap()) {
    auto unregisterTokenSHV = unregisterTokenPV.getSmallHermesValue();
    if (!unregisterTokenSHV.isUndefined()) {
      this->unregisterToken = WeakRefObjOrSym{runtime, unregisterTokenSHV};
    }
  }

  /// Weak reference to the target object/symbol.
  WeakRefObjOrSym target;

  /// Weak reference to the unregister token. If unregister token is not
  /// provided, or explicitly provided as Undefined, this weak ref is Empty,
  /// and this record can not be unregistered.
  WeakRefObjOrSym unregisterToken{nullptr};

  /// Strong reference to the heldValue.
  GCHermesValue heldValue;
};

/// JSFinalizationRegistry implements the ES12 FinalizationRegistry object.
/// It manages registration and unregistration of cleanup operations that are
/// performed when the targets are garbage collected.
///
/// Per ES16 26.2, each FinalizationRegistry has:
/// - [[CleanupCallback]]: A callable invoked for cleanup
/// - [[Cells]]: A list of records { [[WeakRefTarget]], [[HeldValue]],
///              [[UnregisterToken]] }
///
/// The implementation stores cells in an ArrayStorage of FinalizationRecord
/// objects, which have weak references to targets and unregister tokens, and
/// strong reference to held values.
class JSFinalizationRegistry final : public JSObject {
 public:
  using StorageType = ArrayStorage;
  using StorageIndex = StorageType::size_type;

  static const ObjectVTable vt;

  static constexpr CellKind getCellKind() {
    return CellKind::JSFinalizationRegistryKind;
  }

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::JSFinalizationRegistryKind;
  }

  /// Create a new FinalizationRegistry with prototype property \p parentHandle
  /// and cleanup callback \p cleanupCallback.
  static PseudoHandle<JSFinalizationRegistry> create(
      Runtime &runtime,
      Handle<JSObject> parentHandle,
      Handle<Callable> cleanupCallback);

  /// ES16 9.12 CleanupFinalizationRegistry
  /// Run \ref cleanupCallback_ on each dead registered target.
  static ExecutionStatus cleanup(
      Handle<JSFinalizationRegistry> self,
      Runtime &runtime);

  /// ES16 26.2.3.2 FinalizationRegistry.prototype.register
  /// Register a target to this registry.
  /// \param target The object/symbol to watch (which will be weakly held).
  /// \param heldValue The value passed to cleanup callback.
  /// \param unregisterToken Optional token for unregistration. This can either
  ///  be Undefined, or a object/symbol that will be weakly held.
  /// \return EXCEPTION if fail to allocate \ref cells_ or the new record.
  ///  RETURNED otherwise.
  static ExecutionStatus registerCell(
      Handle<JSFinalizationRegistry> self,
      Runtime &runtime,
      Handle<> target,
      Handle<> heldValue,
      Handle<> unregisterToken);

  /// ES16 26.2.3.3 FinalizationRegistry.prototype.unregister
  /// Unregister all cells with matching token.
  /// \return true if any cells were removed
  /// \param unregisterToken The token to match. This must be Object or
  /// unregistered Symbol.
  bool unregisterCells(Runtime &runtime, Handle<> unregisterToken);

  friend void JSFinalizationRegistryBuildMeta(
      const GCCell *cell,
      Metadata::Builder &mb);

  JSFinalizationRegistry(
      Runtime &runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz,
      Handle<Callable> cleanupCallback)
      : JSObject(runtime, *parent, *clazz),
        cleanupCallback_(runtime, *cleanupCallback, runtime.getHeap()) {
    runtime.addFinalizationRegistry(this);
  }

 private:
  /// Get a free index from the free list. If none is available, return
  /// llvh::None. If there is one, return the index and update \ref nextFree_
  /// to the next free index in the free list.
  OptValue<StorageIndex> useFreeIndex(Runtime &runtime);

  /// Push a freed index onto the free list. The freed slot will store the
  /// previous nextFree_ value (or its own index if there was none, marking
  /// end of list), and nextFree_ will be updated to point to this index.
  void free(Runtime &runtime, StorageIndex freedIndex);

  /// Resize \ref cells_ to numUsedCells_ * 2 if the current occupancy ratio is
  /// lower than kOccupancyRatio. Allocates a new storage array and copies
  /// alive elements to it.
  static void tryShrink(Handle<JSFinalizationRegistry> self, Runtime &runtime);

  /// The cleanup callback [[CleanupCallback]].
  /// This is a GCPointer to ensure it's traced by GC.
  GCPointer<Callable> cleanupCallback_;

  /// Registration cells [[Cells]].
  GCPointer<StorageType> cells_{nullptr};

  /// The most recently freed index. The cell at this index contains a
  /// NativeUInt32:
  /// - If the value equals the index itself, this is the last free slot.
  /// - Otherwise, the value is the index of the next free slot in the list.
  OptValue<StorageType::size_type> nextFree_{llvh::None};

  /// Number of used cells in \ref cells_.
  StorageIndex numUsedCells_{0};

  /// Initial capacity for cells_.
  static constexpr StorageIndex kInitCapacity = 4;

  /// If the ratio of occupied targets to the size of cells_ is smaller than
  /// this ratio, we perform a shrink operation.
  static constexpr double kOccupancyRatio = 0.25;
};

} // namespace vm
} // namespace hermes
