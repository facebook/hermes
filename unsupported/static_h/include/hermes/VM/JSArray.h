/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JSARRAY_H
#define HERMES_VM_JSARRAY_H

#include "hermes/VM/IterationKind.h"
#include "hermes/VM/JSObject.h"
#include "hermes/VM/SegmentedArray.h"

namespace hermes {
namespace vm {

/// A common implementation of "Array-like" objects.
class ArrayImpl : public JSObject {
  using Super = JSObject;
  friend void ArrayImplBuildMeta(const GCCell *cell, Metadata::Builder &mb);

 public:
  static bool classof(const GCCell *cell) {
    return kindInRange(
        cell->getKind(),
        CellKind::ArrayImplKind_first,
        CellKind::ArrayImplKind_last);
  }

  /// @name API for C++ users.
  /// @{
  using size_type = uint32_t;
  /// StorageType is the underlying storage that JSArray uses to put the values
  /// into.
  using StorageType = SegmentedArraySmall;

  /// Resize the internal storage. The ".length" property is not affected. It
  /// does \b NOT check for read-only properties.
  static ExecutionStatus setStorageEndIndex(
      Handle<ArrayImpl> selfHandle,
      Runtime &runtime,
      uint32_t newLength);

  /// Update the element at index \p index. If necessary, the array will be
  /// resized, but if it is an \c JSArray, it's \c .length property will not
  /// be affected.
  /// Note that even though the underlying \c setOwnIndexed() interface defines
  /// failure modes, our concrete implementation can never fail.
  static void setElementAt(
      Handle<ArrayImpl> selfHandle,
      Runtime &runtime,
      size_type index,
      Handle<> value) {
    auto result = _setOwnIndexedImpl(selfHandle, runtime, index, value);
    (void)result;
    assert(
        result != ExecutionStatus::EXCEPTION && *result &&
        "JSArrayImpl::setElementAt() failing");
  }

  /// Update an array element, which must exist in storage. Elements with value
  /// "empty" are assumed to exist for the purpose of this definition. This is
  /// only safe to do for arrays that were created by the caller, can be
  /// extended, are not sealed or frozen, and were never passed to user JS code.
  static void unsafeSetExistingElementAt(
      ArrayImpl *self,
      Runtime &runtime,
      size_type index,
      SmallHermesValue value) {
    // The array must be extendable (and by implication is not frozen or sealed)
    // because we don't know whether the element being set is empty or not.
    assert(!self->flags_.noExtend && "this array cannot be extended");

    assert(
        index >= self->beginIndex_ && index < self->endIndex_ &&
        "array index out of range");
    self->getIndexedStorage(runtime)->set(
        runtime, index - self->beginIndex_, value);
  }

  /// Set the element at index \p index to empty. This does not affect the
  /// storage size or array length.
  /// \return true if the operation succeeded (which is always in this class).
  static bool deleteElementAt(
      Handle<ArrayImpl> selfHandle,
      Runtime &runtime,
      size_type index) {
    return _deleteOwnIndexedImpl(selfHandle, runtime, index);
  }

  /// \return the first index of the array.
  size_type getBeginIndex() const {
    return beginIndex_;
  }

  /// \return 1 + the index of the last element contained in the storage.
  size_type getEndIndex() const {
    return endIndex_;
  }

  /// Return the value at index \p index, or \c empty if the index is not
  /// contained in the storage.
  const SmallHermesValue at(Runtime &runtime, size_type index) const {
    return index >= beginIndex_ && index < endIndex_
        ? getIndexedStorage(runtime)->at(runtime, index - beginIndex_)
        : SmallHermesValue::encodeEmptyValue();
  }

  /// Return the value at index \p index.
  Handle<> handleAt(Runtime &runtime, size_type index) const {
    return runtime.makeHandle(at(runtime, index).unboxToHV(runtime));
  }

  /// Get a pointer to the indexed storage for this array. The returned value
  /// may be null if there is no indexed storage.
  StorageType *getIndexedStorage(PointerBase &base) const {
    return indexedStorage_.get(base);
  }

  /// Set the indexed storage of this array to be \p p. The pointer is allowed
  /// to be null.
  void setIndexedStorage(PointerBase &base, StorageType *p, GC &gc) {
    indexedStorage_.set(base, p, gc);
  }

  /// @}

 protected:
  /// Initialize the object with the supplied parameters.
  /// \param indexedStorage allocated storage for indexed properties. It can be
  ///   nullptr, which implies capacity and size of 0. We rely on this when
  ///   constructing array objects using the JavaScript Array() constructor,
  ///   since we don't know the length in advance.
  /// \tparam NeedsBarriers indicates whether write barriers are needed
  ///   for initializing writes in the constructor.  (In debug builds,
  ///   a claim that they are not necessary is checked dynamically,
  ///   which should find any incorrect specifications.)
  template <typename NeedsBarriers>
  ArrayImpl(
      Runtime &runtime,
      JSObject *parent,
      HiddenClass *clazz,
      NeedsBarriers needsBarriers)
      : JSObject(runtime, parent, clazz, needsBarriers) {
    flags_.indexedStorage = true;
    flags_.fastIndexProperties = true;
  }

  /// Default needsBarriers to Yes.
  ArrayImpl(Runtime &runtime, JSObject *parent, HiddenClass *clazz)
      : ArrayImpl(runtime, parent, clazz, GCPointerBase::YesBarriers()) {}

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
  /// extract its \c PropertyFlags (if necessary checking whether the object is
  /// frozen or sealed).
  /// \return PropertyFlags if the property exists.
  static OptValue<PropertyFlags> _getOwnIndexedPropertyFlagsImpl(
      JSObject *selfObj,
      Runtime &runtime,
      uint32_t index);

  /// \return the range of indexes (end-exclusive) in the array.
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

  /// Set an element in the "indexed storage" of this object. Depending on the
  /// semantics of the "indexed storage" the storage capacity may need to be
  /// expanded (e.g. affecting Array.length), or the write may simply be ignored
  /// (in the case of typed arrays).
  /// \return true if the write succeeded, or false if it was ignored.
  static CallResult<bool> _setOwnIndexedImpl(
      Handle<JSObject> selfHandle,
      Runtime &runtime,
      uint32_t index,
      Handle<> value);

  /// Delete an element in the "indexed storage".
  /// \return 'true' if the element was successfully deleted, or if it was
  ///     outside of the storage range. 'false' if this storage doesn't support
  ///     "holes"/deletion (e.g. typed arrays).
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

  /// Return the value at index \p index, which must be valid.
  const SmallHermesValue unsafeAt(Runtime &runtime, size_type index) const {
    return getIndexedStorage(runtime)->at(runtime, index - beginIndex_);
  }

 private:
  /// The first index contained in the storage.
  uint32_t beginIndex_{0};
  /// One past the last index contained in the storage.
  uint32_t endIndex_{0};
  /// The indexed storage for this array.
  GCPointer<StorageType> indexedStorage_;
};

class Arguments final : public ArrayImpl {
  using Super = ArrayImpl;

 public:
  static const ObjectVTable vt;

  // We need one more slot for the '.length' property.
  static const PropStorage::size_type NAMED_PROPERTY_SLOTS =
      Super::NAMED_PROPERTY_SLOTS + 1;

  static constexpr CellKind getCellKind() {
    return CellKind::ArgumentsKind;
  }
  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::ArgumentsKind;
  }

  /// Create an instance of Arguments, with size and capacity equal to \p length
  /// and a property "length" initialized to that value.
  static CallResult<Handle<Arguments>> create(
      Runtime &runtime,
      size_type length,
      Handle<Callable> curFunction,
      bool strictMode);

  Arguments(
      Runtime &runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz)
      : ArrayImpl(runtime, *parent, *clazz) {}
};

class JSArray final : public ArrayImpl {
  using Super = ArrayImpl;

  // Object needs to be able to call setLength.
  friend class JSObject;

  static constexpr SlotIndex jsArrayPropertyCount() {
    return numOverlapSlots<JSArray>() + NAMED_PROPERTY_SLOTS;
  }

  static constexpr inline SlotIndex lengthPropIndex() {
    return jsArrayPropertyCount() - 1;
  }

 public:
  static const ObjectVTable vt;

  // We need one more slot for the '.length' property.
  static const PropStorage::size_type NAMED_PROPERTY_SLOTS =
      Super::NAMED_PROPERTY_SLOTS + 1;

  /// Construct an instance of the hidden class describing the layout of JSArray
  /// instances.
  static Handle<HiddenClass> createClass(
      Runtime &runtime,
      Handle<JSObject> prototypeHandle);

  static constexpr CellKind getCellKind() {
    return CellKind::JSArrayKind;
  }
  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::JSArrayKind;
  }

  static uint32_t getLength(const JSArray *self, PointerBase &pb) {
    return getDirectSlotValue<lengthPropIndex()>(self).getNumber(pb);
  }

  /// Create an instance of Array, with [[Prototype]] initialized with
  /// \p prototypeHandle, with capacity for \p capacity elements and actual size
  /// \p length.
  static CallResult<Handle<JSArray>> create(
      Runtime &runtime,
      Handle<JSObject> prototypeHandle,
      Handle<HiddenClass> classHandle,
      size_type capacity = 0,
      size_type length = 0);

  static CallResult<Handle<JSArray>> create(
      Runtime &runtime,
      Handle<JSObject> prototypeHandle,
      size_type capacity,
      size_type length) {
    return create(
        runtime,
        prototypeHandle,
        *prototypeHandle == runtime.arrayPrototype.getObject()
            ? Handle<HiddenClass>::vmcast(&runtime.arrayClass)
            : createClass(runtime, prototypeHandle),
        capacity,
        length);
  }
  static CallResult<Handle<JSArray>> create(
      Runtime &runtime,
      Handle<JSObject> prototypeHandle) {
    return create(runtime, prototypeHandle, 0, 0);
  }

  /// Create an instance of Array, using the standard array prototype, with
  /// capacity for \p capacity elements and actual size \p length.
  static CallResult<Handle<JSArray>>
  create(Runtime &runtime, size_type capacity, size_type length);

  /// A convenience method for setting the \c .length property of the array.
  /// It performs the necessary checks and updates the property. It could fail
  /// if the property is not writable or if there are read-only index-like
  /// properties which cannot be deleted (see the spec for details).
  static CallResult<bool> setLengthProperty(
      Handle<JSArray> selfHandle,
      Runtime &runtime,
      uint32_t newValue,
      PropOpFlags opFlags = PropOpFlags{}) {
    // TODO: optimize this now that we know the index of the property slot.
    return putNamed_RJS(
        selfHandle,
        runtime,
        Predefined::getSymbolID(Predefined::length),
        runtime.makeHandle(HermesValue::encodeNumberValue(newValue)));
  }

  template <typename NeedsBarrier>
  JSArray(
      Runtime &runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz,
      NeedsBarrier needsBarrier)
      : ArrayImpl(runtime, *parent, *clazz, needsBarrier) {}

 private:
  /// A helper to update the named '.length' property.
  static void
  putLength(JSArray *self, Runtime &runtime, SmallHermesValue newLength) {
    setDirectSlotValue<lengthPropIndex()>(self, newLength, runtime.getHeap());
  }

  /// Update the JavaScript '.length' property, which also resizes the array.
  /// The writability of the property \b MUST already have been checked.
  /// If not sure, use \c putNamed().
  static CallResult<bool> setLength(
      Handle<JSArray> selfHandle,
      Runtime &runtime,
      Handle<> newLength,
      PropOpFlags opFlags) LLVM_NO_SANITIZE("float-cast-overflow");

  /// Update the JavaScript '.length' property, which also resizes the array.
  /// The writability of the property \b MUST already have been checked.
  /// If not sure, use \c putNamed().
  static CallResult<bool> setLength(
      Handle<JSArray> selfHandle,
      Runtime &runtime,
      uint32_t newLength,
      PropOpFlags opFlags);
};

/// The ArrayIterator class, which is used to iterate from 0 to the array's
/// length.
/// ES6.0 22.1.5.
class JSArrayIterator : public JSObject {
  using Super = JSObject;

  friend void JSArrayIteratorBuildMeta(
      const GCCell *cell,
      Metadata::Builder &mb);

 public:
  static const ObjectVTable vt;

  static constexpr CellKind getCellKind() {
    return CellKind::JSArrayIteratorKind;
  }
  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::JSArrayIteratorKind;
  }

  static PseudoHandle<JSArrayIterator>
  create(Runtime &runtime, Handle<JSObject> array, IterationKind iterationKind);

  /// Iterate to the next element and return.
  static CallResult<HermesValue> nextElement(
      Handle<JSArrayIterator> self,
      Runtime &runtime);

 public:
  JSArrayIterator(
      Runtime &runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz,
      Handle<JSObject> iteratedObject,
      IterationKind iterationKind)
      : JSObject(runtime, *parent, *clazz),
        iteratedObject_(runtime, *iteratedObject, runtime.getHeap()),
        iterationKind_(iterationKind) {}

 private:
  /// [[IteratedObject]]
  /// This is null if iteration has been completed.
  GCPointer<JSObject> iteratedObject_;

  /// [[ArrayIteratorNextIndex]]
  uint64_t nextIndex_{0};

  /// [[ArrayIterationKind]]
  IterationKind iterationKind_;
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_JSARRAY_H
