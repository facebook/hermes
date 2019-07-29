/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
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
  using StorageType = BigStorage;

  /// Resize the internal storage. The ".length" property is not affected. It
  /// does \b NOT check for read-only properties.
  static ExecutionStatus setStorageEndIndex(
      Handle<ArrayImpl> selfHandle,
      Runtime *runtime,
      uint32_t newLength);

  /// Update the element at index \p index. If necessary, the array will be
  /// resized, but if it is an \c JSArray, it's \c .length property will not
  /// be affected.
  /// Note that even though the underlying \c setOwnIndexed() interface defines
  /// failure modes, our concrete implementation can never fail.
  static void setElementAt(
      Handle<ArrayImpl> selfHandle,
      Runtime *runtime,
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
      Runtime *runtime,
      size_type index,
      HermesValue value) {
    // The array must be extendable (and by implication is not frozen or sealed)
    // because we don't know whether the element being set is empty or not.
    assert(!self->flags_.noExtend && "this array cannot be extended");

    assert(
        index >= self->beginIndex_ && index < self->endIndex_ &&
        "array index out of range");
    self->indexedStorage_.getNonNull(runtime)
        ->at(index - self->beginIndex_)
        .set(value, &runtime->getHeap());
  }

  /// Set the element at index \p index to empty. This does not affect the
  /// storage size or array length.
  /// \return true if the operation succeeded (which is always in this class).
  static bool deleteElementAt(
      Handle<ArrayImpl> selfHandle,
      Runtime *runtime,
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
  const HermesValue at(Runtime *runtime, size_type index) const {
    return index >= beginIndex_ && index < endIndex_
        ? indexedStorage_.getNonNull(runtime)->at(index - beginIndex_)
        : HermesValue::encodeEmptyValue();
  }

  /// Return the value at index \p index.
  Handle<> handleAt(Runtime *runtime, size_type index) const {
    return runtime->makeHandle(at(runtime, index));
  }
  /// @}

 protected:
  /// Initialize the object with the supplied parameters.
  /// \param indexedStorage allocated storage for indexed properties. It can be
  ///   nullptr, which implies capacity and size of 0. We rely on this when
  ///   constructing array objects using the JavaScript Array() constructor,
  ///   since we don't know the length in advance.
  /// \param needsBarrier indicates whether write barriers are needed
  ///   for initializating writes in the constructor.  (In debug builds,
  ///   a claim that they are not necessary is checked dynamically,
  ///   which should find any incorrect specifications.)
  template <typename NeedsBarriers>
  ArrayImpl(
      Runtime *runtime,
      const VTable *vt,
      JSObject *parent,
      HiddenClass *clazz,
      StorageType *indexedStorage,
      NeedsBarriers needsBarriers)
      : JSObject(runtime, vt, parent, clazz, needsBarriers),
        indexedStorage_(
            runtime,
            indexedStorage,
            &runtime->getHeap(),
            needsBarriers) {
    flags_.indexedStorage = true;
    flags_.fastIndexProperties = true;
  }

  /// Default needsBarriers to Yes.
  ArrayImpl(
      Runtime *runtime,
      const VTable *vt,
      JSObject *parent,
      HiddenClass *clazz,
      StorageType *indexedStorage)
      : ArrayImpl(
            runtime,
            vt,
            parent,
            clazz,
            indexedStorage,
            GCPointerBase::YesBarriers()) {}

  /// Check whether property with index \p index exists in indexed storage and
  /// \return true if it does.
  static bool
  _haveOwnIndexedImpl(JSObject *selfObj, Runtime *runtime, uint32_t index);

  /// Check whether property with index \p index exists in indexed storage and
  /// extract its \c PropertyFlags (if necessary checking whether the object is
  /// frozen or sealed).
  /// \return PropertyFlags if the property exists.
  static OptValue<PropertyFlags> _getOwnIndexedPropertyFlagsImpl(
      JSObject *selfObj,
      Runtime *runtime,
      uint32_t index);

  /// \return the range of indexes (end-exclusive) in the array.
  static std::pair<uint32_t, uint32_t> _getOwnIndexedRangeImpl(
      JSObject *selfObj,
      Runtime *runtime);

  /// Obtain an element from the "indexed storage" of this object. The storage
  /// itself is implementation dependent.
  /// \return the value of the element or "empty" if there is no such element.
  static HermesValue
  _getOwnIndexedImpl(JSObject *self, Runtime *runtime, uint32_t index);

  /// Set an element in the "indexed storage" of this object. Depending on the
  /// semantics of the "indexed storage" the storage capacity may need to be
  /// expanded (e.g. affecting Array.length), or the write may simply be ignored
  /// (in the case of typed arrays).
  /// \return true if the write succeeded, or fals if it was ignored.
  static CallResult<bool> _setOwnIndexedImpl(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      uint32_t index,
      Handle<> value);

  /// Delete an element in the "indexed storage".
  /// \return 'true' if the element was successfully deleted, or if it was
  ///     outside of the storage range. 'false' if this storage doesn't support
  ///     "holes"/deletion (e.g. typed arrays).
  static bool _deleteOwnIndexedImpl(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      uint32_t index);

  /// Check whether all indexed properties satisfy the requirement specified by
  /// \p mode. Either whether they are all non-configurable, or whether they are
  /// all both non-configurable and non-writable.
  static bool _checkAllOwnIndexedImpl(
      JSObject *selfObj,
      Runtime *runtime,
      ObjectVTable::CheckAllOwnIndexedMode mode);

  /// Return the value at index \p index, which must be valid.
  const HermesValue unsafeAt(Runtime *runtime, size_type index) const {
    return indexedStorage_.getNonNull(runtime)->at(index - beginIndex_);
  }

 private:
  /// The first index contained in the storage.
  uint32_t beginIndex_{0};
  /// One past the last index contained in the storage.
  uint32_t endIndex_{0};
  /// The indexed property storage. It can be nullptr, if both its capacity and
  /// size are 0.
  GCPointer<StorageType> indexedStorage_;
};

class Arguments final : public ArrayImpl {
  using Super = ArrayImpl;

 public:
  static ObjectVTable vt;

  // We need one more slot for the '.length' property.
  static const PropStorage::size_type NEEDED_PROPERTY_SLOTS =
      Super::NEEDED_PROPERTY_SLOTS + 1;

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::ArgumentsKind;
  }

  /// Create an instance of Arguments, with size and capacity equal to \p length
  /// and a property "length" initialized to that value.
  static CallResult<HermesValue> create(
      Runtime *runtime,
      size_type length,
      Handle<Callable> curFunction,
      bool strictMode);

 private:
  Arguments(
      Runtime *runtime,
      JSObject *parent,
      HiddenClass *clazz,
      StorageType *indexedStorage)
      : ArrayImpl(runtime, &vt.base, parent, clazz, indexedStorage) {}
};

class JSArray final : public ArrayImpl {
  using Super = ArrayImpl;

 public:
  static ObjectVTable vt;

  // We need one more slot for the '.length' property.
  static const PropStorage::size_type NEEDED_PROPERTY_SLOTS =
      Super::NEEDED_PROPERTY_SLOTS + 1;

  /// Construct an instance of the hidden class describing the layout of JSArray
  /// instances.
  static Handle<HiddenClass> createClass(
      Runtime *runtime,
      Handle<JSObject> prototypeHandle);

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::ArrayKind;
  }

  static uint32_t getLength(const JSArray *self) {
    return self->shadowLength_;
  }

  /// Create an instance of Array, with [[Prototype]] initialized with
  /// \p prototypeHandle, with capacity for \p capacity elements and actual size
  /// \p length.
  static CallResult<HermesValue> create(
      Runtime *runtime,
      Handle<JSObject> prototypeHandle,
      Handle<HiddenClass> classHandle,
      size_type capacity = 0,
      size_type length = 0);

  static CallResult<HermesValue> create(
      Runtime *runtime,
      Handle<JSObject> prototypeHandle,
      size_type capacity,
      size_type length) {
    return create(
        runtime,
        prototypeHandle,
        *prototypeHandle == runtime->arrayPrototype.getObject()
            ? Handle<HiddenClass>::vmcast(&runtime->arrayClass)
            : createClass(runtime, prototypeHandle),
        capacity,
        length);
  }
  static CallResult<HermesValue> create(
      Runtime *runtime,
      Handle<JSObject> prototypeHandle) {
    return create(runtime, prototypeHandle, 0, 0);
  }

  /// Create an instance of Array, using the standard array prototype, with
  /// capacity for \p capacity elements and actual size \p length.
  static CallResult<PseudoHandle<JSArray>>
  create(Runtime *runtime, size_type capacity, size_type length);

  /// A convenience method for setting the \c .length property of the array.
  /// It performs the necessary checks and updates the property. It could fail
  /// if the property is not writable or if there are read-only index-like
  /// properties which cannot be deleted (see the spec for details).
  static CallResult<bool> setLengthProperty(
      Handle<JSArray> selfHandle,
      Runtime *runtime,
      uint32_t newValue,
      PropOpFlags opFlags = PropOpFlags{}) {
    // TODO: optimize this now that we know the index of the property slot.
    return putNamed_RJS(
        selfHandle,
        runtime,
        Predefined::getSymbolID(Predefined::length),
        runtime->makeHandle(HermesValue::encodeNumberValue(newValue)));
  }

 private:
  // Object needs to be able to call setLength.
  friend class JSObject;

  enum : SlotIndex { LengthPropIndex = 0, JSArrayPropertyCount = 1 };

  /// A copy of the ".length" property. We compare every putComputed()
  /// index against ".length", and extracting it from property storage every
  /// time would be too slow.
  uint32_t shadowLength_{0};

  template <typename NeedsBarrier>
  JSArray(
      Runtime *runtime,
      JSObject *parent,
      HiddenClass *clazz,
      StorageType *indexedStorage,
      NeedsBarrier needsBarrier)
      : ArrayImpl(
            runtime,
            &vt.base,
            parent,
            clazz,
            indexedStorage,
            needsBarrier) {}

  /// A helper to update the named '.length' property.
  static void putLength(JSArray *self, Runtime *runtime, uint32_t newLength) {
    self->shadowLength_ = newLength;

    namedSlotRef(self, runtime, LengthPropIndex)
        .setNonPtr(HermesValue::encodeNumberValue(newLength));
  }

  /// Update the JavaScript '.length' property, which also resizes the array.
  /// The writability of the property \b MUST already have been checked.
  /// If not sure, use \c putNamed().
  static CallResult<bool> setLength(
      Handle<JSArray> selfHandle,
      Runtime *runtime,
      Handle<> newLength,
      PropOpFlags opFlags) LLVM_NO_SANITIZE("float-cast-overflow");

  /// Update the JavaScript '.length' property, which also resizes the array.
  /// The writability of the property \b MUST already have been checked.
  /// If not sure, use \c putNamed().
  static CallResult<bool> setLength(
      Handle<JSArray> selfHandle,
      Runtime *runtime,
      uint32_t newLength,
      PropOpFlags opFlags);
};

/// The ArrayIterator class, which is used to iterate from 0 to the array's
/// length.
/// ES6.0 22.1.5.
class JSArrayIterator : public JSObject {
  using Super = JSObject;

  friend void ArrayIteratorBuildMeta(const GCCell *cell, Metadata::Builder &mb);

 public:
  static ObjectVTable vt;

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::ArrayIteratorKind;
  }

  static CallResult<HermesValue>
  create(Runtime *runtime, Handle<JSObject> array, IterationKind iterationKind);

  /// Iterate to the next element and return.
  static CallResult<HermesValue> nextElement(
      Handle<JSArrayIterator> self,
      Runtime *runtime);

 private:
  JSArrayIterator(
      Runtime *runtime,
      JSObject *parent,
      HiddenClass *clazz,
      JSObject *iteratedObject,
      IterationKind iterationKind)
      : JSObject(runtime, &vt.base, parent, clazz),
        iteratedObject_(runtime, iteratedObject, &runtime->getHeap()),
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
