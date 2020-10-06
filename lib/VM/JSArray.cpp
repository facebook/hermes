/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSArray.h"

#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/JSTypedArray.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/StringView.h"

#include "llvh/Support/Debug.h"
#define DEBUG_TYPE "serialize"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
// class ArrayImpl

void ArrayImplBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<ArrayImpl>());
  ObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const ArrayImpl *>(cell);
  // This edge has to be called "elements" in order for Chrome to attribute
  // the size of the indexed storage as part of total usage of "JS Arrays".
  mb.addField("elements", &self->indexedStorage_);
}

void ArrayImpl::_snapshotAddEdgesImpl(
    GCCell *cell,
    GC *gc,
    HeapSnapshot &snap) {
  auto *const self = vmcast<ArrayImpl>(cell);
  // Add the super type's edges too.
  JSObject::_snapshotAddEdgesImpl(self, gc, snap);
  if (!self->indexedStorage_) {
    return;
  }
  auto *const indexedStorage =
      self->indexedStorage_.getNonNull(gc->getPointerBase());
  const auto len = self->endIndex_ - self->beginIndex_;
  for (uint32_t i = 0; i < len; i++) {
    const auto &elem = indexedStorage->at(i);
    const llvh::Optional<HeapSnapshot::NodeID> elemID = gc->getSnapshotID(elem);
    if (!elemID) {
      continue;
    }
    snap.addIndexedEdge(HeapSnapshot::EdgeType::Element, i, elemID.getValue());
  }
}

#ifdef HERMESVM_SERIALIZE
ArrayImpl::ArrayImpl(Deserializer &d, const VTable *vt) : JSObject(d, vt) {
  beginIndex_ = d.readInt<uint32_t>();
  endIndex_ = d.readInt<uint32_t>();
  d.readRelocation(&indexedStorage_, RelocationKind::GCPointer);
}

void serializeArrayImpl(
    Serializer &s,
    const GCCell *cell,
    unsigned overlapSlots) {
  auto *self = vmcast<const ArrayImpl>(cell);
  JSObject::serializeObjectImpl(s, cell, overlapSlots);
  s.writeInt<uint32_t>(self->beginIndex_);
  s.writeInt<uint32_t>(self->endIndex_);
  s.writeRelocation(self->indexedStorage_.get(s.getRuntime()));
}
#endif

bool ArrayImpl::_haveOwnIndexedImpl(
    JSObject *selfObj,
    Runtime *runtime,
    uint32_t index) {
  auto *self = vmcast<ArrayImpl>(selfObj);

  // Check whether the index is within the storage.
  if (index >= self->beginIndex_ && index < self->endIndex_)
    return !self->indexedStorage_.getNonNull(runtime)
                ->at(index - self->beginIndex_)
                .isEmpty();

  return false;
}

OptValue<PropertyFlags> ArrayImpl::_getOwnIndexedPropertyFlagsImpl(
    JSObject *selfObj,
    Runtime *runtime,
    uint32_t index) {
  auto *self = vmcast<ArrayImpl>(selfObj);

  // Check whether the index is within the storage.
  if (index >= self->beginIndex_ && index < self->endIndex_ &&
      !self->indexedStorage_.getNonNull(runtime)
           ->at(index - self->beginIndex_)
           .isEmpty()) {
    PropertyFlags indexedElementFlags{};
    indexedElementFlags.enumerable = 1;
    indexedElementFlags.writable = 1;
    indexedElementFlags.configurable = 1;

    if (LLVM_UNLIKELY(self->flags_.sealed)) {
      indexedElementFlags.configurable = 0;
      if (LLVM_UNLIKELY(self->flags_.frozen))
        indexedElementFlags.writable = 0;
    }

    return indexedElementFlags;
  }

  return llvh::None;
}

std::pair<uint32_t, uint32_t> ArrayImpl::_getOwnIndexedRangeImpl(
    JSObject *selfObj,
    Runtime *runtime) {
  auto *self = vmcast<ArrayImpl>(selfObj);
  return {self->beginIndex_, self->endIndex_};
}

HermesValue ArrayImpl::_getOwnIndexedImpl(
    JSObject *selfObj,
    Runtime *runtime,
    uint32_t index) {
  return vmcast<ArrayImpl>(selfObj)->at(runtime, index);
}

ExecutionStatus ArrayImpl::setStorageEndIndex(
    Handle<ArrayImpl> selfHandle,
    Runtime *runtime,
    uint32_t newLength) {
  auto *self = selfHandle.get();

  if (LLVM_UNLIKELY(
          newLength > self->beginIndex_ &&
          newLength - self->beginIndex_ > StorageType::maxElements())) {
    return runtime->raiseRangeError("Out of memory for array elements");
  }

  // If indexedStorage hasn't even been allocated.
  if (LLVM_UNLIKELY(!self->indexedStorage_)) {
    if (newLength == 0) {
      return ExecutionStatus::RETURNED;
    }
    auto arrRes = StorageType::create(runtime, newLength, newLength);
    if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    auto newStorage = runtime->makeHandle<StorageType>(std::move(*arrRes));
    selfHandle->indexedStorage_.set(
        runtime, newStorage.get(), &runtime->getHeap());
    selfHandle->beginIndex_ = 0;
    selfHandle->endIndex_ = newLength;
    return ExecutionStatus::RETURNED;
  }

  auto beginIndex = self->beginIndex_;

  {
    NoAllocScope scope{runtime};
    auto *const indexedStorage = self->indexedStorage_.getNonNull(runtime);

    if (newLength <= beginIndex) {
      // the new length is prior to beginIndex, clearing the storage.
      selfHandle->endIndex_ = beginIndex;
      // Remove the storage. If this array grows again it can be re-allocated.
      self->indexedStorage_.setNull(&runtime->getHeap());
      return ExecutionStatus::RETURNED;
    } else if (newLength - beginIndex <= indexedStorage->capacity()) {
      selfHandle->endIndex_ = newLength;
      StorageType::resizeWithinCapacity(
          indexedStorage, runtime, newLength - beginIndex);
      return ExecutionStatus::RETURNED;
    }
  }

  auto indexedStorage = runtime->makeMutableHandle(selfHandle->indexedStorage_);

  if (StorageType::resize(indexedStorage, runtime, newLength - beginIndex) ==
      ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  selfHandle->endIndex_ = newLength;
  selfHandle->indexedStorage_.set(
      runtime, indexedStorage.get(), &runtime->getHeap());
  return ExecutionStatus::RETURNED;
}

CallResult<bool> ArrayImpl::_setOwnIndexedImpl(
    Handle<JSObject> selfHandle,
    Runtime *runtime,
    uint32_t index,
    Handle<> value) {
  auto *self = vmcast<ArrayImpl>(selfHandle.get());
  auto beginIndex = self->beginIndex_;
  auto endIndex = self->endIndex_;

  if (LLVM_UNLIKELY(self->flags_.frozen))
    return false;

  // Check whether the index is within the storage.
  if (LLVM_LIKELY(index >= beginIndex && index < endIndex)) {
    self->indexedStorage_.getNonNull(runtime)
        ->at(index - beginIndex)
        .set(value.get(), &runtime->getHeap());
    return true;
  }

  // If indexedStorage hasn't even been allocated.
  if (LLVM_UNLIKELY(!self->indexedStorage_)) {
    // Allocate storage with capacity for 4 elements and length 1.
    auto arrRes = StorageType::create(runtime, 4, 1);
    if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    auto newStorage = runtime->makeHandle<StorageType>(std::move(*arrRes));

    self = vmcast<ArrayImpl>(selfHandle.get());

    self->indexedStorage_.set(runtime, newStorage.get(), &runtime->getHeap());
    self->beginIndex_ = index;
    self->endIndex_ = index + 1;
    newStorage->at(0).set(value.get(), &runtime->getHeap());
    return true;
  }

  {
    NoAllocScope scope{runtime};
    auto *const indexedStorage = self->indexedStorage_.getNonNull(runtime);

    // Can we do it without reallocation for sure?
    if (index >= endIndex && index - beginIndex < indexedStorage->capacity()) {
      self->endIndex_ = index + 1;
      StorageType::resizeWithinCapacity(
          indexedStorage, runtime, index - beginIndex + 1);
      // self shouldn't have moved since there haven't been any allocations.
      indexedStorage->at(index - beginIndex)
          .set(value.get(), &runtime->getHeap());
      return true;
    }
  }

  auto indexedStorageHandle = runtime->makeMutableHandle(self->indexedStorage_);
  // We only shift an array if the shift amount is within the limit.
  constexpr uint32_t shiftLimit = (1 << 20);

  // Is the array empty?
  if (LLVM_UNLIKELY(endIndex == beginIndex)) {
    if (StorageType::resize(indexedStorageHandle, runtime, 1) ==
        ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    indexedStorageHandle->at(0).set(
        value.getHermesValue(), &runtime->getHeap());
    self = vmcast<ArrayImpl>(selfHandle.get());
    self->beginIndex_ = index;
    self->endIndex_ = index + 1;
  } else if (LLVM_UNLIKELY(
                 (index > endIndex && index - endIndex > shiftLimit) ||
                 (index < beginIndex && beginIndex - index > shiftLimit))) {
    // The new index is too far away from the current index range.
    // Shifting will lead to a very large allocation.
    // This is likely a misuse of the array (e.g. use array as an object).
    // In this case, we should just treat the index access as
    // a property access.
    auto vr = valueToSymbolID(
        runtime, runtime->makeHandle(HermesValue::encodeNumberValue(index)));
    assert(
        vr != ExecutionStatus::EXCEPTION &&
        "valueToIdentifier() failed for uint32_t value");

    if (LLVM_UNLIKELY(
            JSObject::defineNewOwnProperty(
                selfHandle,
                runtime,
                **vr,
                PropertyFlags::defaultNewNamedPropertyFlags(),
                value) == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    // self->indexedStorage_ is unmodified, we should return directly.
    return true;
  } else if (index >= endIndex) {
    // Extending to the right.
    if (StorageType::resize(
            indexedStorageHandle, runtime, index - beginIndex + 1) ==
        ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    self = vmcast<ArrayImpl>(selfHandle.get());
    self->endIndex_ = index + 1;
    indexedStorageHandle->at(index - beginIndex)
        .set(value.get(), &runtime->getHeap());
  } else {
    // Extending to the left. 'index' will become the new 'beginIndex'.
    assert(index < beginIndex);

    if (StorageType::resizeLeft(
            indexedStorageHandle,
            runtime,
            indexedStorageHandle->size() + beginIndex - index) ==
        ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    self = vmcast<ArrayImpl>(selfHandle.get());
    self->beginIndex_ = index;
    indexedStorageHandle->at(0).set(value.get(), &runtime->getHeap());
  }

  // Update the potentially changed pointer.
  self->indexedStorage_.set(
      runtime, indexedStorageHandle.get(), &runtime->getHeap());
  return true;
}

bool ArrayImpl::_deleteOwnIndexedImpl(
    Handle<JSObject> selfHandle,
    Runtime *runtime,
    uint32_t index) {
  auto *self = vmcast<ArrayImpl>(selfHandle.get());

  if (index >= self->beginIndex_ && index < self->endIndex_) {
    auto &elem = self->indexedStorage_.getNonNull(runtime)->at(
        index - self->beginIndex_);

    // Cannot delete indexed elements if we are sealed.
    if (LLVM_UNLIKELY(self->flags_.sealed))
      if (!elem.isEmpty())
        return false;

    elem.setNonPtr(HermesValue::encodeEmptyValue(), &runtime->getHeap());
  }

  return true;
}

bool ArrayImpl::_checkAllOwnIndexedImpl(
    JSObject *selfObj,
    Runtime *runtime,
    ObjectVTable::CheckAllOwnIndexedMode /*mode*/
) {
  auto *self = vmcast<ArrayImpl>(selfObj);

  // If we have any indexed properties at all, they don't satisfy the
  // requirements.
  for (uint32_t i = 0, e = self->endIndex_ - self->beginIndex_; i != e; ++i) {
    if (!self->indexedStorage_.getNonNull(runtime)->at(i).isEmpty())
      return false;
  }
  return true;
}

//===----------------------------------------------------------------------===//
// class Arguments

ObjectVTable Arguments::vt{
    VTable(
        CellKind::ArgumentsKind,
        cellSize<Arguments>(),
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr, // externalMemorySize
        VTable::HeapSnapshotMetadata{HeapSnapshot::NodeType::Object,
                                     nullptr,
                                     Arguments::_snapshotAddEdgesImpl,
                                     nullptr,
                                     nullptr}),
    Arguments::_getOwnIndexedRangeImpl,
    Arguments::_haveOwnIndexedImpl,
    Arguments::_getOwnIndexedPropertyFlagsImpl,
    Arguments::_getOwnIndexedImpl,
    Arguments::_setOwnIndexedImpl,
    Arguments::_deleteOwnIndexedImpl,
    Arguments::_checkAllOwnIndexedImpl,
};

void ArgumentsBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<Arguments>());
  ArrayImplBuildMeta(cell, mb);
}

#ifdef HERMESVM_SERIALIZE
Arguments::Arguments(Deserializer &d) : ArrayImpl(d, &vt.base) {}

void ArgumentsSerialize(Serializer &s, const GCCell *cell) {
  serializeArrayImpl(s, cell, JSObject::numOverlapSlots<Arguments>());
  s.endObject(cell);
}

void ArgumentsDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::ArgumentsKind && "Expected Arguments");
  void *mem = d.getRuntime()->alloc(cellSize<Arguments>());
  auto *cell = new (mem) Arguments(d);
  d.endObject(cell);
}
#endif

CallResult<Handle<Arguments>> Arguments::create(
    Runtime *runtime,
    size_type length,
    Handle<Callable> curFunction,
    bool strictMode) {
  auto arrRes = StorageType::create(runtime, length);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto indexedStorage = runtime->makeHandle<StorageType>(std::move(*arrRes));

  JSObjectAlloc<Arguments> mem{runtime};
  auto selfHandle = mem.initToHandle(new (mem) Arguments(
      runtime,
      runtime->objectPrototypeRawPtr,
      runtime->getHiddenClassForPrototypeRaw(
          runtime->objectPrototypeRawPtr,
          numOverlapSlots<Arguments>() + ANONYMOUS_PROPERTY_SLOTS),
      *indexedStorage));

  Arguments::setStorageEndIndex(selfHandle, runtime, length);

  PropertyFlags pf{};
  namespace P = Predefined;

/// Adds a property to the object in \p OBJ_HANDLE.  \p SYMBOL provides its name
/// as a \c Predefined enum value, and its value is  rooted in \p HANDLE.  If
/// property definition fails, the exceptional execution status will be
/// propogated to the outer function.
#define DEFINE_PROP(OBJ_HANDLE, SYMBOL, HANDLE)                            \
  do {                                                                     \
    auto status = JSObject::defineNewOwnProperty(                          \
        OBJ_HANDLE, runtime, Predefined::getSymbolID(SYMBOL), pf, HANDLE); \
    if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {             \
      return ExecutionStatus::EXCEPTION;                                   \
    }                                                                      \
  } while (false)

  // Define the length property.
  pf.enumerable = 0;
  pf.writable = 1;
  pf.configurable = 1;

  DEFINE_PROP(
      selfHandle,
      P::length,
      runtime->makeHandle(HermesValue::encodeDoubleValue(length)));

  DEFINE_PROP(
      selfHandle, P::SymbolIterator, Handle<>(&runtime->arrayPrototypeValues));

  if (strictMode) {
    // Define .callee and .caller properties: throw always in strict mode.
    auto accessor =
        Handle<PropertyAccessor>::vmcast(&runtime->throwTypeErrorAccessor);

    pf.clear();
    pf.enumerable = 0;
    pf.writable = 0;
    pf.configurable = 0;
    pf.accessor = 1;

    DEFINE_PROP(selfHandle, P::callee, accessor);
    DEFINE_PROP(selfHandle, P::caller, accessor);
  } else {
    // Define .callee in non-strict mode to point to the current function.
    // Leave .caller undefined, because it's a non-standard ES extension.
    assert(
        vmisa<Callable>(curFunction.getHermesValue()) &&
        "attempt to reify arguments with a non-callable callee");

    pf.clear();
    pf.enumerable = 0;
    pf.writable = 1;
    pf.configurable = 1;

    DEFINE_PROP(selfHandle, P::callee, curFunction);
  }

  return selfHandle;

#undef DEFINE_PROP
}

//===----------------------------------------------------------------------===//
// class JSArray

ObjectVTable JSArray::vt{
    VTable(
        CellKind::ArrayKind,
        cellSize<JSArray>(),
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr, // externalMemorySize
        VTable::HeapSnapshotMetadata{HeapSnapshot::NodeType::Object,
                                     nullptr,
                                     JSArray::_snapshotAddEdgesImpl,
                                     nullptr,
                                     nullptr}),
    JSArray::_getOwnIndexedRangeImpl,
    JSArray::_haveOwnIndexedImpl,
    JSArray::_getOwnIndexedPropertyFlagsImpl,
    JSArray::_getOwnIndexedImpl,
    JSArray::_setOwnIndexedImpl,
    JSArray::_deleteOwnIndexedImpl,
    JSArray::_checkAllOwnIndexedImpl,
};

void ArrayBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSArray>());
  ArrayImplBuildMeta(cell, mb);
}

#ifdef HERMESVM_SERIALIZE
JSArray::JSArray(Deserializer &d, const VTable *vt) : ArrayImpl(d, vt) {
  shadowLength_ = d.readInt<uint32_t>();
}

void ArraySerialize(Serializer &s, const GCCell *cell) {
  auto *self = vmcast<const JSArray>(cell);
  serializeArrayImpl(s, cell, JSObject::numOverlapSlots<JSArray>());
  s.writeInt<uint32_t>(self->shadowLength_);
  s.endObject(cell);
}

void ArrayDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::ArrayKind && "Expected Array");
  void *mem = d.getRuntime()->alloc(cellSize<JSArray>());
  auto *cell = new (mem) JSArray(d, &JSArray::vt.base);
  d.endObject(cell);
}
#endif

Handle<HiddenClass> JSArray::createClass(
    Runtime *runtime,
    Handle<JSObject> prototypeHandle) {
  Handle<HiddenClass> classHandle{
      runtime,
      runtime->getHiddenClassForPrototypeRaw(
          *prototypeHandle,
          numOverlapSlots<JSArray>() + ANONYMOUS_PROPERTY_SLOTS)};

  PropertyFlags pf{};
  pf.enumerable = 0;
  pf.writable = 1;
  pf.configurable = 0;
  pf.internalSetter = 1;

  auto added = HiddenClass::addProperty(
      classHandle, runtime, Predefined::getSymbolID(Predefined::length), pf);
  assert(
      added != ExecutionStatus::EXCEPTION &&
      "Adding the first properties shouldn't cause overflow");
  assert(
      added->second == lengthPropIndex() && "JSArray.length has invalid index");
  classHandle = added->first;

  assert(
      classHandle->getNumProperties() == jsArrayPropertyCount() &&
      "JSArray class defined with incorrect number of properties");

  return classHandle;
}

CallResult<PseudoHandle<JSArray>> JSArray::create(
    Runtime *runtime,
    Handle<JSObject> prototypeHandle,
    Handle<HiddenClass> classHandle,
    size_type capacity,
    size_type length) {
  assert(length <= capacity && "length must be <= capacity");

  // Allocate property storage with size corresponding to number of properties
  // in the hidden class.
  assert(
      classHandle->getNumProperties() == jsArrayPropertyCount() &&
      "invalid number of properties in JSArray hidden class");

  // Only allocate the storage if capacity is not zero.
  MutableHandle<StorageType> indexedStorage{runtime, nullptr};
  if (capacity) {
    if (LLVM_UNLIKELY(capacity > StorageType::maxElements()))
      return runtime->raiseRangeError("Out of memory for array elements");
    auto arrRes = StorageType::create(runtime, capacity);
    if (arrRes == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    indexedStorage = std::move(*arrRes);
  }

  JSObjectAlloc<JSArray> mem{runtime};
  auto self = mem.initToPseudoHandle(new (mem) JSArray(
      runtime,
      *prototypeHandle,
      *classHandle,
      *indexedStorage,
      GCPointerBase::NoBarriers()));
  putLength(self.get(), runtime, length);

  return self;
}

CallResult<PseudoHandle<JSArray>>
JSArray::create(Runtime *runtime, size_type capacity, size_type length) {
  return JSArray::create(
      runtime,
      Handle<JSObject>::vmcast(&runtime->arrayPrototype),
      Handle<HiddenClass>::vmcast(&runtime->arrayClass),
      capacity,
      length);
}

CallResult<bool> JSArray::setLength(
    Handle<JSArray> selfHandle,
    Runtime *runtime,
    Handle<> newLength,
    PropOpFlags opFlags) {
  // Convert the value to uint32_t.
  double d;
  if (newLength->isNumber()) {
    d = newLength->getNumber();
  } else {
    auto res = toNumber_RJS(runtime, newLength);
    if (res == ExecutionStatus::EXCEPTION)
      return ExecutionStatus::EXCEPTION;
    d = res->getNumber();
  }

  // NOTE: in theory this produces UB, however in practice it is well defined.
  // Regardless what happens in the conversion, 'ulen' can never compare equal
  // to 'd' unless 'd' is really an uint32 number, in which case the conversion
  // would have succeeded.
  // The only way this could fail is if the conversion throws an exception or
  // aborts the application, which is not the case on any platform we are
  // targeting.
  uint32_t ulen = (uint32_t)d;
  if (ulen != d)
    return runtime->raiseRangeError("Invalid array length");

  return setLength(selfHandle, runtime, ulen, opFlags);
}

CallResult<bool> JSArray::setLength(
    Handle<JSArray> selfHandle,
    Runtime *runtime,
    uint32_t newLength,
    PropOpFlags opFlags) {
  // Fast-path: if we are enlarging, do nothing.
  if (LLVM_LIKELY(newLength >= selfHandle->shadowLength_)) {
    putLength(*selfHandle, runtime, newLength);
    return true;
  }

  // Length adjusted to the index of the highest non-deletable property + 1.
  // Nothing smaller than it can be deleted.
  uint32_t adjustedLength = newLength;

  // If we are sealed, we can't shrink past non-empty properties.
  if (LLVM_UNLIKELY(selfHandle->flags_.sealed)) {
    // We must scan backwards looking for a non-empty property. We only have
    // to scan in the intersection between the range of present values and
    // the range between the current length and the new length.
    //              newLength         shadowLength
    //                 |                    |
    //                 +--------------------+
    //       begin                end
    //         |                   |
    //         +-------------------+
    //                 +-----------+
    //                 |           |
    //          lowestScanLen  highestLen
    //

    auto *self = selfHandle.get();
    auto range = _getOwnIndexedRangeImpl(self, runtime);
    uint32_t lowestScanLen = std::max(range.first, newLength);
    uint32_t highestLen = std::min(range.second, self->shadowLength_);

    for (; highestLen > lowestScanLen; --highestLen) {
      if (!self->unsafeAt(runtime, highestLen - 1).isEmpty()) {
        adjustedLength = highestLen;
        break;
      }
    }
  }

  if (LLVM_UNLIKELY(selfHandle->clazz_.getNonNull(runtime)
                        ->getHasIndexLikeProperties())) {
    // Uh-oh. We are making the array smaller and we have index-like named
    // properties, so we may have to delete some of them: the ones greater or
    // equal to 'newLength'.

    // We iterate all named properties to find all index-like ones that need to
    // be deleted. At the same time we keep track of the highest index of a non-
    // deletable property - nothing smaller than that can be deleted because the
    // highest non-deletable would have terminated the deletion process.

    using IndexProp = std::pair<uint32_t, SymbolID>;
    llvh::SmallVector<IndexProp, 8> toBeDeleted;

    GCScope scope{runtime};

    HiddenClass::forEachProperty(
        runtime->makeHandle(selfHandle->clazz_),
        runtime,
        [runtime, &adjustedLength, &toBeDeleted, &scope](
            SymbolID id, NamedPropertyDescriptor desc) {
          GCScopeMarkerRAII marker{scope};
          // If this property is not an integer index, or it doesn't need to be
          // deleted (it is less than 'adjustedLength'), ignore it.
          auto propNameAsIndex = toArrayIndex(
              runtime->getIdentifierTable().getStringView(runtime, id));
          if (!propNameAsIndex || *propNameAsIndex < adjustedLength)
            return;

          if (!desc.flags.configurable) {
            adjustedLength = *propNameAsIndex + 1;
          } else {
            toBeDeleted.push_back({*propNameAsIndex, id});
          }
        });

    // Scan the properties to be deleted in reverse order (to make deletion more
    // efficient) and delete those >= adjustedLength.
    for (auto it = toBeDeleted.rbegin(), e = toBeDeleted.rend(); it != e;
         ++it) {
      if (it->first >= adjustedLength) {
        GCScopeMarkerRAII marker{scope};
        auto cr = JSObject::deleteNamed(selfHandle, runtime, it->second);
        assert(
            cr != ExecutionStatus::EXCEPTION && *cr &&
            "Failed to delete a configurable property");
        (void)cr;
      }
    }
  }

  if (adjustedLength < selfHandle->getEndIndex()) {
    auto cr = setStorageEndIndex(selfHandle, runtime, adjustedLength);
    if (cr == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
  }
  putLength(*selfHandle, runtime, adjustedLength);

  if (adjustedLength != newLength) {
    if (opFlags.getThrowOnError()) {
      return runtime->raiseTypeError(
          TwineChar16("Cannot delete property '") + (adjustedLength - 1) + "'");
    }
    return false;
  }

  return true;
}

//===----------------------------------------------------------------------===//
// class JSArrayIterator

ObjectVTable JSArrayIterator::vt{
    VTable(CellKind::ArrayIteratorKind, cellSize<JSArrayIterator>()),
    JSArrayIterator::_getOwnIndexedRangeImpl,
    JSArrayIterator::_haveOwnIndexedImpl,
    JSArrayIterator::_getOwnIndexedPropertyFlagsImpl,
    JSArrayIterator::_getOwnIndexedImpl,
    JSArrayIterator::_setOwnIndexedImpl,
    JSArrayIterator::_deleteOwnIndexedImpl,
    JSArrayIterator::_checkAllOwnIndexedImpl,
};

void ArrayIteratorBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSArrayIterator>());
  ObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const JSArrayIterator *>(cell);
  mb.addField("iteratedObject", &self->iteratedObject_);
}

#ifdef HERMESVM_SERIALIZE
JSArrayIterator::JSArrayIterator(Deserializer &d) : JSObject(d, &vt.base) {
  d.readRelocation(&iteratedObject_, RelocationKind::GCPointer);
  nextIndex_ = d.readInt<uint64_t>();
  iterationKind_ = (IterationKind)d.readInt<uint8_t>();
}

void ArrayIteratorSerialize(Serializer &s, const GCCell *cell) {
  auto *self = vmcast<const JSArrayIterator>(cell);
  JSObject::serializeObjectImpl(
      s, cell, JSObject::numOverlapSlots<JSArrayIterator>());
  s.writeRelocation(self->iteratedObject_.get(s.getRuntime()));
  s.writeInt<uint64_t>(self->nextIndex_);
  s.writeInt<uint8_t>((uint8_t)self->iterationKind_);
  s.endObject(cell);
}

void ArrayIteratorDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::ArrayIteratorKind && "Expected ArrayIterator");
  void *mem = d.getRuntime()->alloc(cellSize<JSArrayIterator>());
  auto *cell = new (mem) JSArrayIterator(d);
  d.endObject(cell);
}
#endif

PseudoHandle<JSArrayIterator> JSArrayIterator::create(
    Runtime *runtime,
    Handle<JSObject> array,
    IterationKind iterationKind) {
  auto proto = Handle<JSObject>::vmcast(&runtime->arrayIteratorPrototype);

  JSObjectAlloc<JSArrayIterator> mem{runtime};
  return mem.initToPseudoHandle(new (mem) JSArrayIterator(
      runtime,
      *proto,
      runtime->getHiddenClassForPrototypeRaw(
          *proto,
          numOverlapSlots<JSArrayIterator>() + ANONYMOUS_PROPERTY_SLOTS),
      *array,
      iterationKind));
}

/// Iterate to the next element and return.
CallResult<HermesValue> JSArrayIterator::nextElement(
    Handle<JSArrayIterator> self,
    Runtime *runtime) {
  if (!self->iteratedObject_) {
    // 5. If a is undefined, return CreateIterResultObject(undefined, true).
    return createIterResultObject(runtime, Runtime::getUndefinedValue(), true)
        .getHermesValue();
  }

  // 4. Let a be the value of the [[IteratedObject]] internal slot of O.
  Handle<JSObject> a = runtime->makeHandle(self->iteratedObject_);
  // 6. Let index be the value of the [[ArrayIteratorNextIndex]] internal slot
  // of O.
  uint64_t index = self->nextIndex_;

  uint64_t len;
  if (auto ta = Handle<JSTypedArrayBase>::dyn_vmcast(a)) {
    // 8. If a has a [[TypedArrayName]] internal slot, then
    // a. If IsDetachedBuffer(a.[[ViewedArrayBuffer]]) is true,
    //    throw a TypeError exception.
    // b. Let len be the value of O’s [[ArrayLength]] internal slot.
    if (LLVM_UNLIKELY(!ta->attached(runtime))) {
      return runtime->raiseTypeError("TypedArray detached during iteration");
    }
    len = ta->getLength();
  } else {
    // 9. Else,
    // a. Let len be ToLength(Get(a, "length")).
    auto propRes = JSObject::getNamed_RJS(
        a, runtime, Predefined::getSymbolID(Predefined::length));
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    auto lenRes = toLength(runtime, runtime->makeHandle(std::move(*propRes)));
    if (LLVM_UNLIKELY(lenRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    len = lenRes->getNumber();
  }

  if (index >= len) {
    // 10. If index ≥ len, then
    // a. Set the value of the [[IteratedObject]] internal slot of O to
    // undefined.
    self->iteratedObject_.setNull(&runtime->getHeap());
    // b. Return CreateIterResultObject(undefined, true).
    return createIterResultObject(runtime, Runtime::getUndefinedValue(), true)
        .getHermesValue();
  }

  // 11. Set the value of the [[ArrayIteratorNextIndex]] internal slot of O to
  // index+1.
  ++self->nextIndex_;

  auto indexHandle = runtime->makeHandle(HermesValue::encodeNumberValue(index));

  if (self->iterationKind_ == IterationKind::Key) {
    // 12. If itemKind is "key", return CreateIterResultObject(index, false).
    return createIterResultObject(runtime, indexHandle, false).getHermesValue();
  }

  // 13. Let elementKey be ToString(index).
  // 14. Let elementValue be Get(a, elementKey).
  CallResult<PseudoHandle<>> valueRes =
      JSObject::getComputed_RJS(a, runtime, indexHandle);
  if (LLVM_UNLIKELY(valueRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<> valueHandle = runtime->makeHandle(std::move(*valueRes));

  switch (self->iterationKind_) {
    case IterationKind::Key:
      llvm_unreachable("Early return already occurred in Key case");
      return HermesValue::encodeEmptyValue();
    case IterationKind::Value:
      // 16. If itemKind is "value", let result be elementValue.
      return createIterResultObject(runtime, valueHandle, false)
          .getHermesValue();
    case IterationKind::Entry: {
      // 17. b. Let result be CreateArrayFromList(«index, elementValue»).
      auto resultRes = JSArray::create(runtime, 2, 2);
      if (LLVM_UNLIKELY(resultRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      Handle<JSArray> result = runtime->makeHandle(std::move(*resultRes));
      JSArray::setElementAt(result, runtime, 0, indexHandle);
      JSArray::setElementAt(result, runtime, 1, valueHandle);
      // 18. Return CreateIterResultObject(result, false).
      return createIterResultObject(runtime, result, false).getHermesValue();
    }
    case IterationKind::NumKinds:
      llvm_unreachable("Invalid iteration kind");
      return HermesValue::encodeEmptyValue();
  }

  llvm_unreachable("Invalid iteration kind");
  return HermesValue::encodeEmptyValue();
}

} // namespace vm
} // namespace hermes
