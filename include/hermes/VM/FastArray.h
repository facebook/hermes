/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_FASTARRAY_H
#define HERMES_VM_FASTARRAY_H

#include "hermes/VM/IterationKind.h"
#include "hermes/VM/JSObject.h"
#include "hermes/VM/SegmentedArray.h"

namespace hermes {
namespace vm {

/// A simple fast array type intended to be used from statically typed code. For
/// performance, it uses contiguous backing storage, and forbids much of the
/// dynamic behaviour of ordinary JS arrays, like writing to the length or
/// configuring elements.
class FastArray : public JSObject {
  /// The indexed storage for this array.
  GCPointer<ArrayStorageSmall> indexedStorage_;

  friend void FastArrayBuildMeta(const GCCell *cell, Metadata::Builder &mb);

  /// \return the index at which we know the length property will be stored.
  static constexpr inline SlotIndex lengthPropIndex() {
    // It will be in the first slot after the overlap slots.
    return numOverlapSlots<FastArray>();
  }

 public:
  static const ObjectVTable vt;

  static constexpr CellKind getCellKind() {
    return CellKind::FastArrayKind;
  }

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::FastArrayKind;
  }

  /// \return the length of this FastArray, by reading it from the known length
  /// property slot.
  uint32_t getLength(PointerBase &pb) const {
    return getDirectSlotValue<lengthPropIndex()>(this).getNumber(pb);
  }

  /// \return the value at index \p index, or throw if the index is not
  /// within bounds
  CallResult<SmallHermesValue> at(Runtime &runtime, size_t index) const {
    auto *storage = indexedStorage_.getNonNull(runtime);
    if (index >= getLength(runtime))
      return runtime.raiseRangeError("Array index out of bounds");
    return storage->at(index);
  }

  /// \return the value at \p index, where it is known to be within bounds.
  SmallHermesValue unsafeAt(Runtime &runtime, size_t index) const {
    auto *storage = indexedStorage_.getNonNull(runtime);
    assert(index < getLength(runtime));
    return storage->at(index);
  }

  /// Set the value at \p index to \p val, where it is known to be within
  /// bounds.
  ExecutionStatus
  unsafeSet(Runtime &runtime, size_t index, SmallHermesValue val) {
    auto *storage = indexedStorage_.getNonNull(runtime);
    assert(index < getLength(runtime));
    storage->set(index, val, runtime.getHeap());
    return ExecutionStatus::RETURNED;
  }

  /// Set the value at \p index to \p val, or throw if the index is not within
  /// bounds.
  ExecutionStatus set(Runtime &runtime, size_t index, SmallHermesValue val) {
    auto *storage = indexedStorage_.getNonNull(runtime);
    if (index >= indexedStorage_.getNonNull(runtime)->size())
      return runtime.raiseRangeError("Array index out of bounds");
    storage->set(index, val, runtime.getHeap());
    return ExecutionStatus::RETURNED;
  }

  /// Push the element \p val onto the end of the storage, and increase the
  /// length by 1.
  static ExecutionStatus
  push(Handle<FastArray> self, Runtime &runtime, Handle<> val);

  /// Construct an instance of the hidden class describing the layout of JSArray
  /// instances.
  static Handle<HiddenClass> createClass(
      Runtime &runtime,
      Handle<JSObject> prototypeHandle);

  /// Create a new FastArray with the given capacity. Note that storage will be
  /// allocated even if the capacity is zero, to maintain indexedStorage_ as
  /// non-null.
  static CallResult<Handle<FastArray>> create(
      Runtime &runtime,
      size_t capacity = 4);

  template <typename NeedsBarriers>
  FastArray(
      Runtime &runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz,
      NeedsBarriers needsBarriers)
      : JSObject(runtime, *parent, *clazz, needsBarriers) {
    flags_.indexedStorage = true;
    flags_.fastIndexProperties = true;
    flags_.noExtend = true;
    flags_.sealed = true;
  }

 private:
  /// Set the length of this FastArray to \p newLength.
  void setLength(Runtime &runtime, SmallHermesValue newLength) {
    // TODO: Potentially optimise this by knowing it's a SMI.
    setDirectSlotValue<lengthPropIndex()>(this, newLength, runtime.getHeap());
  }

#ifdef HERMES_MEMORY_INSTRUMENTATION
  /// Adds the special indexed element edges from this array to its backing
  /// storage.
  static void _snapshotAddEdgesImpl(GCCell *cell, GC &gc, HeapSnapshot &snap);
#endif

  /// Check whether property with index \p index exists in indexed storage and
  /// \return true if it does.
  static bool
  _haveOwnIndexedImpl(JSObject *selfObj, Runtime &runtime, uint32_t index);

  /// Check whether property with index \p index exists in indexed storage and
  /// extract its \c PropertyFlags.
  /// \return PropertyFlags if the property exists.
  static OptValue<PropertyFlags> _getOwnIndexedPropertyFlagsImpl(
      JSObject *selfObj,
      Runtime &runtime,
      uint32_t index);

  /// \return the range of indices (end-exclusive) in the array.
  static std::pair<uint32_t, uint32_t> _getOwnIndexedRangeImpl(
      JSObject *selfObj,
      Runtime &runtime);

  /// Obtain an element from the "indexed storage" of this object. The storage
  /// itself is implementation dependent.
  /// \return the value of the element or "empty" if there is no such element.
  static HermesValue _getOwnIndexedImpl(
      PseudoHandle<JSObject> self,
      Runtime &runtime,
      uint32_t index);

  /// Attempts to set an element in the indexed storage. Currently, this always
  /// throws, as setting elements in a FastArray is only supported using
  /// specialised operations in typed mode.
  static CallResult<bool> _setOwnIndexedImpl(
      Handle<JSObject> selfHandle,
      Runtime &runtime,
      uint32_t index,
      Handle<> value);

  /// Attempts to delete an element in the FastArray. FastArray does not
  /// actually support element deletion so this always returns false.
  static bool _deleteOwnIndexedImpl(
      Handle<JSObject> selfHandle,
      Runtime &runtime,
      uint32_t index);

  /// Check whether all indexed properties satisfy the requirement specified by
  /// \p mode. Either whether they are all non-configurable, or whether they are
  /// all both non-configurable and non-writable.
  static bool _checkAllOwnIndexedImpl(
      JSObject *selfObj,
      Runtime &runtime,
      ObjectVTable::CheckAllOwnIndexedMode mode);
};

} // namespace vm
} // namespace hermes
#endif
