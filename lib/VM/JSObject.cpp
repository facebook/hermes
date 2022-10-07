/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSObject.h"

#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/HostModel.h"
#include "hermes/VM/InternalProperty.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/JSProxy.h"
#include "hermes/VM/NativeState.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/PropertyAccessor.h"

#include "llvh/ADT/SmallSet.h"
#pragma GCC diagnostic push

#ifdef HERMES_COMPILER_SUPPORTS_WSHORTEN_64_TO_32
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif
namespace hermes {
namespace vm {

const ObjectVTable JSObject::vt{
    VTable(
        CellKind::JSObjectKind,
        cellSize<JSObject>(),
        nullptr,
        nullptr,
        nullptr,
        nullptr
#ifdef HERMES_MEMORY_INSTRUMENTATION
        ,
        VTable::HeapSnapshotMetadata {
          HeapSnapshot::NodeType::Object, JSObject::_snapshotNameImpl,
              JSObject::_snapshotAddEdgesImpl, nullptr,
              JSObject::_snapshotAddLocationsImpl
        }
#endif
        ),
    JSObject::_getOwnIndexedRangeImpl,
    JSObject::_haveOwnIndexedImpl,
    JSObject::_getOwnIndexedPropertyFlagsImpl,
    JSObject::_getOwnIndexedImpl,
    JSObject::_setOwnIndexedImpl,
    JSObject::_deleteOwnIndexedImpl,
    JSObject::_checkAllOwnIndexedImpl,
};

void JSObjectBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  // This call is just for debugging and consistency purposes.
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSObject>());

  const auto *self = static_cast<const JSObject *>(cell);
  mb.setVTable(&JSObject::vt);
  mb.addField("parent", &self->parent_);
  mb.addField("class", &self->clazz_);
  mb.addField("propStorage", &self->propStorage_);

  // Declare the direct properties.
  static const char *directPropName[JSObject::DIRECT_PROPERTY_SLOTS] = {
      "directProp0",
      "directProp1",
      "directProp2",
      "directProp3",
      "directProp4"};
  for (unsigned i = mb.getJSObjectOverlapSlots();
       i < JSObject::DIRECT_PROPERTY_SLOTS;
       ++i) {
    mb.addField(directPropName[i], self->directProps() + i);
  }
}

PseudoHandle<JSObject> JSObject::create(
    Runtime &runtime,
    Handle<JSObject> parentHandle) {
  auto *cell = runtime.makeAFixed<JSObject>(
      runtime,
      parentHandle,
      runtime.getHiddenClassForPrototype(
          *parentHandle, numOverlapSlots<JSObject>()),
      GCPointerBase::NoBarriers());
  return JSObjectInit::initToPseudoHandle(runtime, cell);
}

PseudoHandle<JSObject> JSObject::create(Runtime &runtime) {
  return create(runtime, Handle<JSObject>::vmcast(&runtime.objectPrototype));
}

PseudoHandle<JSObject> JSObject::create(
    Runtime &runtime,
    unsigned propertyCount) {
  auto self = create(runtime);

  return runtime.ignoreAllocationFailure(
      JSObject::allocatePropStorage(std::move(self), runtime, propertyCount));
}

PseudoHandle<JSObject> JSObject::create(
    Runtime &runtime,
    Handle<HiddenClass> clazz) {
  auto obj = JSObject::create(runtime, clazz->getNumProperties());
  obj->clazz_.setNonNull(runtime, *clazz, runtime.getHeap());
  // If the hidden class has index like property, we need to clear the fast path
  // flag.
  if (LLVM_UNLIKELY(
          obj->clazz_.getNonNull(runtime)->getHasIndexLikeProperties()))
    obj->flags_.fastIndexProperties = false;
  return obj;
}

PseudoHandle<JSObject> JSObject::create(
    Runtime &runtime,
    Handle<JSObject> parentHandle,
    Handle<HiddenClass> clazz) {
  auto *cell = runtime.makeAFixed<JSObject>(
      runtime, parentHandle, clazz, GCPointerBase::NoBarriers());
  auto obj = JSObjectInit::initToPseudoHandle(runtime, cell);

  obj->clazz_.setNonNull(runtime, *clazz, runtime.getHeap());
  // If the hidden class has index like property, we need to clear the fast path
  // flag.
  if (LLVM_UNLIKELY(
          obj->clazz_.getNonNull(runtime)->getHasIndexLikeProperties()))
    obj->flags_.fastIndexProperties = false;
  return obj;
}

void JSObject::initializeLazyObject(
    Runtime &runtime,
    Handle<JSObject> lazyObject) {
  assert(lazyObject->flags_.lazyObject && "object must be lazy");
  // object is now assumed to be a regular object.
  lazyObject->flags_.lazyObject = 0;

  // only functions can be lazy.
  assert(vmisa<Callable>(lazyObject.get()) && "unexpected lazy object");
  Callable::defineLazyProperties(Handle<Callable>::vmcast(lazyObject), runtime);
}

ObjectID JSObject::getObjectID(JSObject *self, Runtime &runtime) {
  if (LLVM_LIKELY(self->flags_.objectID))
    return self->flags_.objectID;

  // Object ID does not yet exist, get next unique global ID..
  self->flags_.objectID = runtime.generateNextObjectID();
  // Make sure it is not zero.
  if (LLVM_UNLIKELY(!self->flags_.objectID))
    --self->flags_.objectID;
  return self->flags_.objectID;
}

CallResult<PseudoHandle<JSObject>> JSObject::getPrototypeOf(
    PseudoHandle<JSObject> selfHandle,
    Runtime &runtime) {
  if (LLVM_LIKELY(!selfHandle->isProxyObject())) {
    return createPseudoHandle(selfHandle->getParent(runtime));
  }

  return JSProxy::getPrototypeOf(
      runtime.makeHandle(std::move(selfHandle)), runtime);
}

namespace {

CallResult<bool> proxyOpFlags(
    Runtime &runtime,
    PropOpFlags opFlags,
    const char *msg,
    CallResult<bool> res) {
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (!*res && opFlags.getThrowOnError()) {
    return runtime.raiseTypeError(msg);
  }
  return res;
}

} // namespace

CallResult<bool> JSObject::setParent(
    JSObject *self,
    Runtime &runtime,
    JSObject *parent,
    PropOpFlags opFlags) {
  if (LLVM_UNLIKELY(self->isProxyObject())) {
    return proxyOpFlags(
        runtime,
        opFlags,
        "Object is not extensible.",
        JSProxy::setPrototypeOf(
            runtime.makeHandle(self), runtime, runtime.makeHandle(parent)));
  }
  // ES9 9.1.2
  // 4.
  if (self->parent_.get(runtime) == parent)
    return true;
  // 5.
  if (!self->isExtensible()) {
    if (opFlags.getThrowOnError()) {
      return runtime.raiseTypeError("Object is not extensible.");
    } else {
      return false;
    }
  }
  // 6-8. Check for a prototype cycle.
  for (JSObject *cur = parent; cur; cur = cur->parent_.get(runtime)) {
    if (cur == self) {
      if (opFlags.getThrowOnError()) {
        return runtime.raiseTypeError("Prototype cycle detected");
      } else {
        return false;
      }
    } else if (LLVM_UNLIKELY(cur->isProxyObject())) {
      // TODO this branch should also be used for module namespace and
      // immutable prototype exotic objects.
      break;
    }
  }
  // 9.
  self->parent_.set(runtime, parent, runtime.getHeap());
  // 10.
  return true;
}

void JSObject::allocateNewSlotStorage(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    SlotIndex newSlotIndex,
    Handle<> valueHandle) {
  // If it is a direct property, just store the value and we are done.
  if (LLVM_LIKELY(newSlotIndex < DIRECT_PROPERTY_SLOTS)) {
    auto shv = SmallHermesValue::encodeHermesValue(*valueHandle, runtime);
    selfHandle->directProps()[newSlotIndex].set(shv, runtime.getHeap());
    return;
  }

  // Make the slot index relative to the indirect storage.
  newSlotIndex -= DIRECT_PROPERTY_SLOTS;

  // Allocate a new property storage if not already allocated.
  if (LLVM_UNLIKELY(!selfHandle->propStorage_)) {
    // Allocate new storage.
    assert(newSlotIndex == 0 && "allocated slot must be at end");
    auto arrRes = runtime.ignoreAllocationFailure(
        PropStorage::create(runtime, DEFAULT_PROPERTY_CAPACITY));
    selfHandle->propStorage_.setNonNull(
        runtime, vmcast<PropStorage>(arrRes), runtime.getHeap());
  } else if (LLVM_UNLIKELY(
                 newSlotIndex >=
                 selfHandle->propStorage_.getNonNull(runtime)->capacity())) {
    // Reallocate the existing one.
    assert(
        newSlotIndex == selfHandle->propStorage_.getNonNull(runtime)->size() &&
        "allocated slot must be at end");
    auto hnd = runtime.makeMutableHandle(selfHandle->propStorage_);
    PropStorage::resize(hnd, runtime, newSlotIndex + 1);
    selfHandle->propStorage_.setNonNull(runtime, *hnd, runtime.getHeap());
  }

  {
    NoAllocScope scope{runtime};
    auto *const propStorage = selfHandle->propStorage_.getNonNull(runtime);
    if (newSlotIndex >= propStorage->size()) {
      assert(
          newSlotIndex == propStorage->size() &&
          "allocated slot must be at end");
      PropStorage::resizeWithinCapacity(propStorage, runtime, newSlotIndex + 1);
    }
  }
  // This must be done after the call to resizeWithinCapacity, since
  // encodeHermesValue may allocate and cause the ArrayStorage to be trimmed.
  auto shv = SmallHermesValue::encodeHermesValue(*valueHandle, runtime);
  // If we don't need to resize, just store it directly.
  selfHandle->propStorage_.getNonNull(runtime)->set(
      newSlotIndex, shv, runtime.getHeap());
}

CallResult<PseudoHandle<>> JSObject::getNamedPropertyValue_RJS(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    Handle<JSObject> propObj,
    NamedPropertyDescriptor desc) {
  if (LLVM_LIKELY(!desc.flags.accessor))
    return getNamedSlotValue(propObj, runtime, desc);

  // It's now valid to use the Internal variant because we know it's an
  // accessor.
  auto *accessor = vmcast<PropertyAccessor>(
      getNamedSlotValueUnsafe(propObj.get(), runtime, desc).getObject(runtime));
  if (!accessor->getter)
    return createPseudoHandle(HermesValue::encodeUndefinedValue());

  // Execute the accessor on this object.
  return accessor->getter.getNonNull(runtime)->executeCall0(
      runtime.makeHandle(accessor->getter), runtime, selfHandle);
}

CallResult<PseudoHandle<>> JSObject::getComputedPropertyValueInternal_RJS(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    Handle<JSObject> propObj,
    ComputedPropertyDescriptor desc) {
  assert(
      !selfHandle->flags_.proxyObject && !propObj->flags_.proxyObject &&
      "getComputedPropertyValue_RJS cannot be used with proxy objects");

  if (LLVM_LIKELY(!desc.flags.accessor))
    return createPseudoHandle(getComputedSlotValueUnsafe(
        createPseudoHandle(propObj.get()), runtime, desc));

  auto *accessor = vmcast<PropertyAccessor>(getComputedSlotValueUnsafe(
      createPseudoHandle(propObj.get()), runtime, desc));
  if (!accessor->getter)
    return createPseudoHandle(HermesValue::encodeUndefinedValue());

  // Execute the accessor on this object.
  return accessor->getter.getNonNull(runtime)->executeCall0(
      runtime.makeHandle(accessor->getter), runtime, selfHandle);
}

CallResult<PseudoHandle<>> JSObject::getComputedPropertyValue_RJS(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    Handle<JSObject> propObj,
    MutableHandle<SymbolID> &tmpSymbolStorage,
    ComputedPropertyDescriptor desc,
    Handle<> nameValHandle) {
  if (!propObj) {
    return createPseudoHandle(HermesValue::encodeEmptyValue());
  }

  if (LLVM_LIKELY(!desc.flags.proxyObject)) {
    return JSObject::getComputedPropertyValueInternal_RJS(
        selfHandle, runtime, propObj, desc);
  }

  CallResult<Handle<>> keyRes = toPropertyKey(runtime, nameValHandle);
  if (LLVM_UNLIKELY(keyRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  CallResult<bool> hasRes = JSProxy::hasComputed(propObj, runtime, *keyRes);
  if (LLVM_UNLIKELY(hasRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (!*hasRes) {
    return createPseudoHandle(HermesValue::encodeEmptyValue());
  }
  return JSProxy::getComputed(propObj, runtime, *keyRes, selfHandle);
}

CallResult<Handle<JSArray>> JSObject::getOwnPropertyKeys(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    OwnKeysFlags okFlags) {
  assert(
      (okFlags.getIncludeNonSymbols() || okFlags.getIncludeSymbols()) &&
      "Can't exclude symbols and strings");
  if (LLVM_UNLIKELY(
          selfHandle->flags_.lazyObject || selfHandle->flags_.proxyObject)) {
    if (selfHandle->flags_.proxyObject) {
      CallResult<PseudoHandle<JSArray>> proxyRes =
          JSProxy::ownPropertyKeys(selfHandle, runtime, okFlags);
      if (LLVM_UNLIKELY(proxyRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      return runtime.makeHandle(std::move(*proxyRes));
    }
    assert(selfHandle->flags_.lazyObject && "descriptor flags are impossible");
    initializeLazyObject(runtime, selfHandle);
  }

  auto range = getOwnIndexedRange(selfHandle.get(), runtime);

  // Estimate the capacity of the output array.  This estimate is only
  // reasonable for the non-symbol case.
  const uint32_t capacity = okFlags.getIncludeNonSymbols()
      ? (selfHandle->clazz_.getNonNull(runtime)->getNumProperties() +
         range.second - range.first)
      : 0;

  auto arrayRes = JSArray::create(runtime, capacity, 0);
  if (LLVM_UNLIKELY(arrayRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto array = *arrayRes;

  // Optional array of SymbolIDs reported via host object API
  llvh::Optional<Handle<JSArray>> hostObjectSymbols;
  size_t hostObjectSymbolCount = 0;

  // If current object is a host object we need to deduplicate its properties
  llvh::SmallSet<SymbolID::RawType, 16> dedupSet;

  // Output index.
  uint32_t index = 0;

  // Avoid allocating a new handle per element.
  MutableHandle<> tmpHandle{runtime};

  // Number of indexed properties.
  uint32_t numIndexed = 0;

  // Regular properties with names that are array indexes are stashed here, if
  // encountered.
  llvh::SmallVector<uint32_t, 8> indexNames{};

  // Iterate the named properties excluding those which use Symbols.
  if (okFlags.getIncludeNonSymbols()) {
    // Get host object property names
    if (LLVM_UNLIKELY(selfHandle->flags_.hostObject)) {
      assert(
          range.first == range.second &&
          "Host objects cannot own indexed range");
      auto hostSymbolsRes =
          vmcast<HostObject>(selfHandle.get())->getHostPropertyNames();
      if (hostSymbolsRes == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      if ((hostObjectSymbolCount = (**hostSymbolsRes)->getEndIndex()) != 0) {
        Handle<JSArray> hostSymbols = *hostSymbolsRes;
        hostObjectSymbols = std::move(hostSymbols);
      }
    }

    // Iterate the indexed properties.
    GCScopeMarkerRAII marker{runtime};
    for (auto i = range.first; i != range.second; ++i) {
      auto res = getOwnIndexedPropertyFlags(selfHandle.get(), runtime, i);
      if (!res)
        continue;

      // If specified, check whether it is enumerable.
      if (!okFlags.getIncludeNonEnumerable() && !res->enumerable)
        continue;

      tmpHandle = HermesValue::encodeDoubleValue(i);
      JSArray::setElementAt(array, runtime, index++, tmpHandle);
      marker.flush();
    }

    numIndexed = index;

    HiddenClass::forEachProperty(
        runtime.makeHandle(selfHandle->clazz_),
        runtime,
        [&runtime,
         okFlags,
         array,
         hostObjectSymbolCount,
         &index,
         &indexNames,
         &tmpHandle,
         &dedupSet](SymbolID id, NamedPropertyDescriptor desc) {
          if (!isPropertyNamePrimitive(id)) {
            return;
          }

          // If specified, check whether it is enumerable.
          if (!okFlags.getIncludeNonEnumerable()) {
            if (!desc.flags.enumerable)
              return;
          }

          // Host properties might overlap with the ones recognized by the
          // hidden class. If we're dealing with a host object then keep track
          // of hidden class properties for the deduplication purposes.
          if (LLVM_UNLIKELY(hostObjectSymbolCount > 0)) {
            dedupSet.insert(id.unsafeGetRaw());
          }

          // Check if this property is an integer index. If it is, we stash it
          // away to deal with it later. This check should be fast since most
          // property names don't start with a digit.
          auto propNameAsIndex = toArrayIndex(
              runtime.getIdentifierTable().getStringView(runtime, id));
          if (LLVM_UNLIKELY(propNameAsIndex)) {
            indexNames.push_back(*propNameAsIndex);
            return;
          }

          tmpHandle = HermesValue::encodeStringValue(
              runtime.getStringPrimFromSymbolID(id));
          JSArray::setElementAt(array, runtime, index++, tmpHandle);
        });

    // Iterate over HostObject properties and append them to the array. Do not
    // append duplicates.
    if (LLVM_UNLIKELY(hostObjectSymbols)) {
      for (size_t i = 0; i < hostObjectSymbolCount; ++i) {
        assert(
            (*hostObjectSymbols)->at(runtime, i).isSymbol() &&
            "Host object needs to return array of SymbolIDs");
        marker.flush();
        SymbolID id = (*hostObjectSymbols)->at(runtime, i).getSymbol();
        if (dedupSet.count(id.unsafeGetRaw()) == 0) {
          dedupSet.insert(id.unsafeGetRaw());

          assert(
              !InternalProperty::isInternal(id) &&
              "host object returned reserved symbol");
          auto propNameAsIndex = toArrayIndex(
              runtime.getIdentifierTable().getStringView(runtime, id));
          if (LLVM_UNLIKELY(propNameAsIndex)) {
            indexNames.push_back(*propNameAsIndex);
            continue;
          }
          tmpHandle = HermesValue::encodeStringValue(
              runtime.getStringPrimFromSymbolID(id));
          JSArray::setElementAt(array, runtime, index++, tmpHandle);
        }
      }
    }
  }

  // Now iterate the named properties again, including only Symbols.
  // We could iterate only once, if we chose to ignore (and disallow)
  // own properties on HostObjects, as we do with Proxies.
  if (okFlags.getIncludeSymbols()) {
    MutableHandle<SymbolID> idHandle{runtime};
    HiddenClass::forEachProperty(
        runtime.makeHandle(selfHandle->clazz_),
        runtime,
        [&runtime, okFlags, array, &index, &idHandle](
            SymbolID id, NamedPropertyDescriptor desc) {
          if (!isSymbolPrimitive(id)) {
            return;
          }
          // If specified, check whether it is enumerable.
          if (!okFlags.getIncludeNonEnumerable()) {
            if (!desc.flags.enumerable)
              return;
          }
          idHandle = id;
          JSArray::setElementAt(array, runtime, index++, idHandle);
        });
  }

  // The end (exclusive) of the named properties.
  uint32_t endNamed = index;

  // Properly set the length of the array.
  auto cr = JSArray::setLength(
      array, runtime, endNamed + indexNames.size(), PropOpFlags{});
  (void)cr;
  assert(
      cr != ExecutionStatus::EXCEPTION && *cr && "JSArray::setLength() failed");

  // If we have no index-like names, we are done.
  if (LLVM_LIKELY(indexNames.empty()))
    return array;

  // In the unlikely event that we encountered index-like names, we need to sort
  // them and merge them with the real indexed properties. Note that it is
  // guaranteed that there are no clashes.
  std::sort(indexNames.begin(), indexNames.end());

  // Also make space for the new elements by shifting all the named properties
  // to the right. First, resize the array.
  JSArray::setStorageEndIndex(array, runtime, endNamed + indexNames.size());

  // Shift the non-index property names. The region [numIndexed..endNamed) is
  // moved to [numIndexed+indexNames.size()..array->size()).
  // TODO: optimize this by implementing memcpy-like functionality in ArrayImpl.
  for (uint32_t last = endNamed, toLast = array->getEndIndex();
       last != numIndexed;) {
    --last;
    --toLast;
    tmpHandle = array->at(runtime, last).unboxToHV(runtime);
    JSArray::setElementAt(array, runtime, toLast, tmpHandle);
  }

  // Now we need to merge the indexes in indexNames and the array
  // [0..numIndexed). We start from the end and copy the larger element from
  // either array.
  // 1+ the destination position to copy into.
  for (uint32_t toLast = numIndexed + indexNames.size(),
                indexNamesLast = indexNames.size();
       toLast != 0;) {
    if (numIndexed) {
      uint32_t a =
          (uint32_t)array->at(runtime, numIndexed - 1).getNumber(runtime);
      uint32_t b;

      if (indexNamesLast && (b = indexNames[indexNamesLast - 1]) > a) {
        tmpHandle = HermesValue::encodeDoubleValue(b);
        --indexNamesLast;
      } else {
        tmpHandle = HermesValue::encodeDoubleValue(a);
        --numIndexed;
      }
    } else {
      assert(indexNamesLast && "prematurely ran out of source values");
      tmpHandle =
          HermesValue::encodeDoubleValue(indexNames[indexNamesLast - 1]);
      --indexNamesLast;
    }

    --toLast;
    JSArray::setElementAt(array, runtime, toLast, tmpHandle);
  }

  return array;
}

/// Convert a value to string unless already converted
/// \param nameValHandle [Handle<>] the value to convert
/// \param str [MutableHandle<StringPrimitive>] the string is stored
///   there. Must be initialized to null initially.
#define LAZY_TO_STRING(runtime, nameValHandle, str)       \
  do {                                                    \
    if (!str) {                                           \
      auto status = toString_RJS(runtime, nameValHandle); \
      assert(                                             \
          status != ExecutionStatus::EXCEPTION &&         \
          "toString() of primitive cannot fail");         \
      str = status->get();                                \
    }                                                     \
  } while (0)

/// Convert a value to an identifier unless already converted
/// \param nameValHandle [Handle<>] the value to convert
/// \param id [SymbolID] the identifier is stored there. Must be initialized
///   to INVALID_IDENTIFIER_ID initially.
#define LAZY_TO_IDENTIFIER(runtime, nameValHandle, id)          \
  do {                                                          \
    if (id.isInvalid()) {                                       \
      CallResult<Handle<SymbolID>> idRes =                      \
          valueToSymbolID(runtime, nameValHandle);              \
      if (LLVM_UNLIKELY(idRes == ExecutionStatus::EXCEPTION)) { \
        return ExecutionStatus::EXCEPTION;                      \
      }                                                         \
      id = **idRes;                                             \
    }                                                           \
  } while (0)

/// Convert a value to array index, if possible.
/// \param nameValHandle [Handle<>] the value to convert
/// \param str [MutableHandle<StringPrimitive>] the string is stored
///   there. Must be initialized to null initially.
/// \param arrayIndex [OptValue<uint32_t>] the array index is stored
///   there.
#define TO_ARRAY_INDEX(runtime, nameValHandle, str, arrayIndex) \
  do {                                                          \
    arrayIndex = toArrayIndexFastPath(*nameValHandle);          \
    if (!arrayIndex && !nameValHandle->isSymbol()) {            \
      LAZY_TO_STRING(runtime, nameValHandle, str);              \
      arrayIndex = toArrayIndex(runtime, str);                  \
    }                                                           \
  } while (0)

/// \return true if the flags of a new property make it suitable for indexed
///   storage. All new indexed properties are enumerable, writable and
///   configurable and have no accessors.
static bool canNewPropertyBeIndexed(DefinePropertyFlags dpf) {
  return dpf.setEnumerable && dpf.enumerable && dpf.setWritable &&
      dpf.writable && dpf.setConfigurable && dpf.configurable &&
      !dpf.setSetter && !dpf.setGetter;
}

struct JSObject::Helper {
 public:
  LLVM_ATTRIBUTE_ALWAYS_INLINE
  static ObjectFlags &flags(JSObject *self) {
    return self->flags_;
  }

  LLVM_ATTRIBUTE_ALWAYS_INLINE
  static OptValue<PropertyFlags>
  getOwnIndexedPropertyFlags(JSObject *self, Runtime &runtime, uint32_t index) {
    return JSObject::getOwnIndexedPropertyFlags(self, runtime, index);
  }

  LLVM_ATTRIBUTE_ALWAYS_INLINE
  static NamedPropertyDescriptor &castToNamedPropertyDescriptorRef(
      ComputedPropertyDescriptor &desc) {
    return desc.castToNamedPropertyDescriptorRef();
  }
};

namespace {

/// ES5.1 8.12.1.

/// A helper which takes a SymbolID which caches the conversion of
/// nameValHandle if it's needed.  It should be default constructed,
/// and may or may not be set.  This has been measured to be a useful
/// perf win.  Note that always_inline seems to be ignored on static
/// methods, so this function has to be local to the cpp file in order
/// to be inlined for the perf win.
LLVM_ATTRIBUTE_ALWAYS_INLINE
inline CallResult<bool> getOwnComputedPrimitiveDescriptorImpl(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    Handle<> nameValHandle,
    JSObject::IgnoreProxy ignoreProxy,
    SymbolID &id,
    MutableHandle<SymbolID> &tmpSymbolStorage,
    ComputedPropertyDescriptor &desc) {
  assert(
      !nameValHandle->isObject() &&
      "nameValHandle passed to "
      "getOwnComputedPrimitiveDescriptor "
      "cannot be an object");

  // Try the fast paths first if we have "fast" index properties and the
  // property name is an obvious index.
  if (auto arrayIndex = toArrayIndexFastPath(*nameValHandle)) {
    if (JSObject::Helper::flags(*selfHandle).fastIndexProperties) {
      auto res = JSObject::Helper::getOwnIndexedPropertyFlags(
          selfHandle.get(), runtime, *arrayIndex);
      if (res) {
        // This a valid array index, residing in our indexed storage.
        desc.flags = *res;
        desc.flags.indexed = 1;
        desc.slot = *arrayIndex;
        return true;
      }

      // This a valid array index, but we don't have it in our indexed storage,
      // and we don't have index-like named properties.
      return false;
    }

    if (!selfHandle->getClass(runtime)->getHasIndexLikeProperties() &&
        !selfHandle->isHostObject() && !selfHandle->isLazy() &&
        !selfHandle->isProxyObject()) {
      // Early return to handle the case where an object definitely has no
      // index-like properties. This avoids allocating a new StringPrimitive and
      // uniquing it below.
      return false;
    }
  }

  // Convert the string to a SymbolID
  LAZY_TO_IDENTIFIER(runtime, nameValHandle, id);

  // Look for a named property with this name.
  if (JSObject::getOwnNamedDescriptor(
          selfHandle,
          runtime,
          id,
          JSObject::Helper::castToNamedPropertyDescriptorRef(desc))) {
    return true;
  }

  if (LLVM_LIKELY(
          !JSObject::Helper::flags(*selfHandle).indexedStorage &&
          !selfHandle->isLazy() && !selfHandle->isProxyObject())) {
    return false;
  }
  MutableHandle<StringPrimitive> strPrim{runtime};

  // If we have indexed storage, perform potentially expensive conversions
  // to array index and check it.
  if (JSObject::Helper::flags(*selfHandle).indexedStorage) {
    // If the name is a valid integer array index, store it here.
    OptValue<uint32_t> arrayIndex;

    // Try to convert the property name to an array index.
    TO_ARRAY_INDEX(runtime, nameValHandle, strPrim, arrayIndex);

    if (arrayIndex) {
      auto res = JSObject::Helper::getOwnIndexedPropertyFlags(
          selfHandle.get(), runtime, *arrayIndex);
      if (res) {
        desc.flags = *res;
        desc.flags.indexed = 1;
        desc.slot = *arrayIndex;
        return true;
      }
    }
    return false;
  }

  if (selfHandle->isLazy()) {
    JSObject::initializeLazyObject(runtime, selfHandle);
    return JSObject::getOwnComputedPrimitiveDescriptor(
        selfHandle,
        runtime,
        nameValHandle,
        ignoreProxy,
        tmpSymbolStorage,
        desc);
  }

  assert(selfHandle->isProxyObject() && "descriptor flags are impossible");
  if (ignoreProxy == JSObject::IgnoreProxy::Yes) {
    return false;
  }
  return JSProxy::getOwnProperty(
      selfHandle, runtime, nameValHandle, desc, nullptr);
}

} // namespace

CallResult<bool> JSObject::getOwnComputedPrimitiveDescriptor(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    Handle<> nameValHandle,
    JSObject::IgnoreProxy ignoreProxy,
    MutableHandle<SymbolID> &tmpSymbolStorage,
    ComputedPropertyDescriptor &desc) {
  SymbolID id{};

  return getOwnComputedPrimitiveDescriptorImpl(
      selfHandle,
      runtime,
      nameValHandle,
      ignoreProxy,
      id,
      tmpSymbolStorage,
      desc);
}

CallResult<bool> JSObject::getOwnComputedDescriptor(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    Handle<> nameValHandle,
    MutableHandle<SymbolID> &tmpSymbolStorage,
    ComputedPropertyDescriptor &desc) {
  auto converted = toPropertyKeyIfObject(runtime, nameValHandle);
  if (LLVM_UNLIKELY(converted == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return JSObject::getOwnComputedPrimitiveDescriptor(
      selfHandle, runtime, *converted, IgnoreProxy::No, tmpSymbolStorage, desc);
}

CallResult<bool> JSObject::getOwnComputedDescriptor(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    Handle<> nameValHandle,
    MutableHandle<SymbolID> &tmpSymbolStorage,
    ComputedPropertyDescriptor &desc,
    MutableHandle<> &valueOrAccessor) {
  auto converted = toPropertyKeyIfObject(runtime, nameValHandle);
  if (LLVM_UNLIKELY(converted == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // The proxy is ignored here so we can avoid calling
  // JSProxy::getOwnProperty twice on proxies, since
  // getOwnComputedPrimitiveDescriptor doesn't pass back the
  // valueOrAccessor.
  CallResult<bool> res = JSObject::getOwnComputedPrimitiveDescriptor(
      selfHandle,
      runtime,
      *converted,
      IgnoreProxy::Yes,
      tmpSymbolStorage,
      desc);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  if (*res) {
    // This is safe because we passed IgnoreProxy::Yes above,
    // meaning that this will return false in proxy cases.
    valueOrAccessor = getComputedSlotValueUnsafe(
        createPseudoHandle(selfHandle.get()), runtime, desc);
    return true;
  }
  if (LLVM_UNLIKELY(selfHandle->isProxyObject())) {
    return JSProxy::getOwnProperty(
        selfHandle, runtime, nameValHandle, desc, &valueOrAccessor);
  }
  return false;
}

JSObject *JSObject::getNamedDescriptorUnsafe(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    SymbolID name,
    PropertyFlags expectedFlags,
    NamedPropertyDescriptor &desc) {
  if (findProperty(selfHandle, runtime, name, expectedFlags, desc))
    return *selfHandle;

  // Check here for host object flag.  This means that "normal" own
  // properties above win over host-defined properties, but there's no
  // cost imposed on own property lookups.  This should do what we
  // need in practice, and we can define host vs js property
  // disambiguation however we want.  This is here in order to avoid
  // impacting perf for the common case where an own property exists
  // in normal storage.
  if (LLVM_UNLIKELY(selfHandle->flags_.hostObject)) {
    desc.flags.hostObject = true;
    desc.flags.writable = true;
    desc.slot = name.unsafeGetRaw();
    return *selfHandle;
  }

  if (LLVM_UNLIKELY(selfHandle->flags_.lazyObject)) {
    assert(
        !selfHandle->flags_.proxyObject &&
        "Proxy objects should never be lazy");
    // Initialize the object and perform the lookup again.
    JSObject::initializeLazyObject(runtime, selfHandle);

    if (findProperty(selfHandle, runtime, name, expectedFlags, desc))
      return *selfHandle;
  }

  if (LLVM_UNLIKELY(selfHandle->flags_.proxyObject)) {
    desc.flags.proxyObject = true;
    desc.slot = name.unsafeGetRaw();
    return *selfHandle;
  }

  if (selfHandle->parent_) {
    MutableHandle<JSObject> mutableSelfHandle{
        runtime, selfHandle->parent_.getNonNull(runtime)};

    do {
      // Check the most common case first, at the cost of some code duplication.
      if (LLVM_LIKELY(
              !mutableSelfHandle->flags_.lazyObject &&
              !mutableSelfHandle->flags_.hostObject &&
              !mutableSelfHandle->flags_.proxyObject)) {
      findProp:
        if (findProperty(
                mutableSelfHandle,
                runtime,
                name,
                PropertyFlags::invalid(),
                desc)) {
          assert(
              !selfHandle->flags_.proxyObject &&
              "Proxy object parents should never have own properties");
          return *mutableSelfHandle;
        }
      } else if (LLVM_UNLIKELY(mutableSelfHandle->flags_.lazyObject)) {
        JSObject::initializeLazyObject(runtime, mutableSelfHandle);
        goto findProp;
      } else if (LLVM_UNLIKELY(mutableSelfHandle->flags_.hostObject)) {
        desc.flags.hostObject = true;
        desc.flags.writable = true;
        desc.slot = name.unsafeGetRaw();
        return *mutableSelfHandle;
      } else {
        assert(
            mutableSelfHandle->flags_.proxyObject &&
            "descriptor flags are impossible");
        desc.flags.proxyObject = true;
        desc.slot = name.unsafeGetRaw();
        return *mutableSelfHandle;
      }
    } while ((mutableSelfHandle = mutableSelfHandle->parent_.get(runtime)));
  }

  return nullptr;
}

ExecutionStatus JSObject::getComputedPrimitiveDescriptor(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    Handle<> nameValHandle,
    MutableHandle<JSObject> &propObj,
    MutableHandle<SymbolID> &tmpSymbolStorage,
    ComputedPropertyDescriptor &desc) {
  assert(
      !nameValHandle->isObject() &&
      "nameValHandle passed to "
      "getComputedPrimitiveDescriptor cannot "
      "be an object");

  propObj = selfHandle.get();

  SymbolID id{};

  GCScopeMarkerRAII marker{runtime};
  do {
    // A proxy is ignored here so we can check the bit later and
    // return it back to the caller for additional processing.

    Handle<JSObject> loopHandle = propObj;

    CallResult<bool> res = getOwnComputedPrimitiveDescriptorImpl(
        loopHandle,
        runtime,
        nameValHandle,
        IgnoreProxy::Yes,
        id,
        tmpSymbolStorage,
        desc);
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (*res) {
      return ExecutionStatus::RETURNED;
    }

    if (LLVM_UNLIKELY(propObj->flags_.hostObject)) {
      desc.flags.hostObject = true;
      desc.flags.writable = true;
      desc.slot = id.unsafeGetRaw();
      tmpSymbolStorage = id;
      return ExecutionStatus::RETURNED;
    }
    if (LLVM_UNLIKELY(propObj->flags_.proxyObject)) {
      desc.flags.proxyObject = true;
      desc.slot = id.unsafeGetRaw();
      tmpSymbolStorage = id;
      return ExecutionStatus::RETURNED;
    }
    // This isn't a proxy, so use the faster getParent() instead of
    // getPrototypeOf.
    propObj = propObj->getParent(runtime);
    // Flush at the end of the loop to allow first iteration to be as fast as
    // possible.
    marker.flush();
  } while (propObj);
  return ExecutionStatus::RETURNED;
}

ExecutionStatus JSObject::getComputedDescriptor(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    Handle<> nameValHandle,
    MutableHandle<JSObject> &propObj,
    MutableHandle<SymbolID> &tmpSymbolStorage,
    ComputedPropertyDescriptor &desc) {
  auto converted = toPropertyKeyIfObject(runtime, nameValHandle);
  if (LLVM_UNLIKELY(converted == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return getComputedPrimitiveDescriptor(
      selfHandle, runtime, *converted, propObj, tmpSymbolStorage, desc);
}

CallResult<PseudoHandle<>> JSObject::getNamedWithReceiver_RJS(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    SymbolID name,
    Handle<> receiver,
    PropOpFlags opFlags,
    PropertyCacheEntry *cacheEntry) {
  NamedPropertyDescriptor desc;
  // Locate the descriptor. propObj contains the object which may be anywhere
  // along the prototype chain.
  JSObject *propObj = getNamedDescriptorUnsafe(selfHandle, runtime, name, desc);
  if (!propObj) {
    if (LLVM_UNLIKELY(opFlags.getMustExist())) {
      return runtime.raiseReferenceError(
          TwineChar16("Property '") +
          runtime.getIdentifierTable().getStringViewForDev(runtime, name) +
          "' doesn't exist");
    }
    return createPseudoHandle(HermesValue::encodeUndefinedValue());
  }

  if (LLVM_LIKELY(
          !desc.flags.accessor && !desc.flags.hostObject &&
          !desc.flags.proxyObject)) {
    // Populate the cache if requested.
    if (cacheEntry && !propObj->getClass(runtime)->isDictionaryNoCache()) {
      cacheEntry->clazz = propObj->getClassGCPtr();
      cacheEntry->slot = desc.slot;
    }
    return createPseudoHandle(
        getNamedSlotValueUnsafe(propObj, runtime, desc).unboxToHV(runtime));
  }

  if (desc.flags.accessor) {
    auto *accessor = vmcast<PropertyAccessor>(
        getNamedSlotValueUnsafe(propObj, runtime, desc).getPointer(runtime));
    if (!accessor->getter)
      return createPseudoHandle(HermesValue::encodeUndefinedValue());

    // Execute the accessor on this object.
    return Callable::executeCall0(
        runtime.makeHandle(accessor->getter), runtime, receiver);
  } else if (desc.flags.hostObject) {
    auto res = vmcast<HostObject>(propObj)->get(name);
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return createPseudoHandle(*res);
  } else {
    assert(desc.flags.proxyObject && "descriptor flags are impossible");
    return JSProxy::getNamed(
        runtime.makeHandle(propObj), runtime, name, receiver);
  }
}

CallResult<PseudoHandle<>> JSObject::getNamedOrIndexed(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    SymbolID name,
    PropOpFlags opFlags) {
  if (LLVM_UNLIKELY(selfHandle->flags_.indexedStorage)) {
    // Note that getStringView can be satisfied without materializing the
    // Identifier.
    const auto strView =
        runtime.getIdentifierTable().getStringView(runtime, name);
    if (auto nameAsIndex = toArrayIndex(strView)) {
      return getComputed_RJS(
          selfHandle,
          runtime,
          runtime.makeHandle(HermesValue::encodeNumberValue(*nameAsIndex)));
    }
    // Here we have indexed properties but the symbol was not index-like.
    // Fall through to getNamed().
  }
  return getNamed_RJS(selfHandle, runtime, name, opFlags);
}

CallResult<PseudoHandle<>> JSObject::getComputedWithReceiver_RJS(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    Handle<> nameValHandle,
    Handle<> receiver) {
  // Try the fast-path first: no "index-like" properties and the "name" already
  // is a valid integer index.
  if (selfHandle->flags_.fastIndexProperties) {
    if (auto arrayIndex = toArrayIndexFastPath(*nameValHandle)) {
      // Do we have this value present in our array storage? If so, return it.
      PseudoHandle<> ourValue =
          createPseudoHandle(getOwnIndexed(selfHandle, runtime, *arrayIndex));
      if (LLVM_LIKELY(!ourValue->isEmpty()))
        return ourValue;
    }
  }

  // If nameValHandle is an object, we should convert it to string now,
  // because toString may have side-effect, and we want to do this only
  // once.
  auto converted = toPropertyKeyIfObject(runtime, nameValHandle);
  if (LLVM_UNLIKELY(converted == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto nameValPrimitiveHandle = *converted;

  ComputedPropertyDescriptor desc;

  // Locate the descriptor. propObj contains the object which may be anywhere
  // along the prototype chain.
  MutableHandle<JSObject> propObj{runtime};
  MutableHandle<SymbolID> tmpPropNameStorage{runtime};
  if (LLVM_UNLIKELY(
          getComputedPrimitiveDescriptor(
              selfHandle,
              runtime,
              nameValPrimitiveHandle,
              propObj,
              tmpPropNameStorage,
              desc) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  if (!propObj)
    return createPseudoHandle(HermesValue::encodeUndefinedValue());

  if (LLVM_LIKELY(
          !desc.flags.accessor && !desc.flags.hostObject &&
          !desc.flags.proxyObject)) {
    return createPseudoHandle(getComputedSlotValueUnsafe(
        createPseudoHandle(propObj.get()), runtime, desc));
  }

  if (desc.flags.accessor) {
    auto *accessor = vmcast<PropertyAccessor>(getComputedSlotValueUnsafe(
        createPseudoHandle(propObj.get()), runtime, desc));
    if (!accessor->getter)
      return createPseudoHandle(HermesValue::encodeUndefinedValue());

    // Execute the accessor on this object.
    return accessor->getter.getNonNull(runtime)->executeCall0(
        runtime.makeHandle(accessor->getter), runtime, receiver);
  } else if (desc.flags.hostObject) {
    SymbolID id{};
    LAZY_TO_IDENTIFIER(runtime, nameValPrimitiveHandle, id);
    auto propRes = vmcast<HostObject>(propObj.get())->get(id);
    if (propRes == ExecutionStatus::EXCEPTION)
      return ExecutionStatus::EXCEPTION;
    return createPseudoHandle(*propRes);
  } else {
    assert(desc.flags.proxyObject && "descriptor flags are impossible");
    CallResult<Handle<>> key = toPropertyKey(runtime, nameValPrimitiveHandle);
    if (key == ExecutionStatus::EXCEPTION)
      return ExecutionStatus::EXCEPTION;
    return JSProxy::getComputed(propObj, runtime, *key, receiver);
  }
}

CallResult<bool> JSObject::hasNamed(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    SymbolID name) {
  NamedPropertyDescriptor desc;
  JSObject *propObj = getNamedDescriptorUnsafe(selfHandle, runtime, name, desc);
  if (propObj == nullptr) {
    return false;
  }
  if (LLVM_UNLIKELY(desc.flags.proxyObject)) {
    return JSProxy::hasNamed(runtime.makeHandle(propObj), runtime, name);
  }
  return true;
}

CallResult<bool> JSObject::hasNamedOrIndexed(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    SymbolID name) {
  if (LLVM_UNLIKELY(selfHandle->flags_.indexedStorage)) {
    const auto strView =
        runtime.getIdentifierTable().getStringView(runtime, name);
    if (auto nameAsIndex = toArrayIndex(strView)) {
      if (haveOwnIndexed(selfHandle.get(), runtime, *nameAsIndex)) {
        return true;
      }
      if (selfHandle->flags_.fastIndexProperties) {
        return false;
      }
    }
    // Here we have indexed properties but the symbol was not stored in the
    // indexedStorage.
    // Fall through to getNamed().
  }
  return hasNamed(selfHandle, runtime, name);
}

CallResult<bool> JSObject::hasComputed(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    Handle<> nameValHandle) {
  // Try the fast-path first: no "index-like" properties and the "name" already
  // is a valid integer index.
  if (selfHandle->flags_.fastIndexProperties) {
    if (auto arrayIndex = toArrayIndexFastPath(*nameValHandle)) {
      // Do we have this value present in our array storage? If so, return true.
      if (haveOwnIndexed(selfHandle.get(), runtime, *arrayIndex)) {
        return true;
      }
    }
  }

  // If nameValHandle is an object, we should convert it to string now,
  // because toString may have side-effect, and we want to do this only
  // once.
  auto converted = toPropertyKeyIfObject(runtime, nameValHandle);
  if (LLVM_UNLIKELY(converted == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto nameValPrimitiveHandle = *converted;

  ComputedPropertyDescriptor desc;
  MutableHandle<SymbolID> tmpPropNameStorage{runtime};
  MutableHandle<JSObject> propObj{runtime};
  if (getComputedPrimitiveDescriptor(
          selfHandle,
          runtime,
          nameValPrimitiveHandle,
          propObj,
          tmpPropNameStorage,
          desc) == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  if (!propObj) {
    return false;
  }
  if (LLVM_UNLIKELY(desc.flags.proxyObject)) {
    CallResult<Handle<>> key = toPropertyKey(runtime, nameValPrimitiveHandle);
    if (key == ExecutionStatus::EXCEPTION)
      return ExecutionStatus::EXCEPTION;
    return JSProxy::hasComputed(propObj, runtime, *key);
  }
  // For compatibility with polyfills we want to pretend that all HostObject
  // properties are "own" properties in 'in'. Since there is no way to check for
  // a HostObject property, we must always assume success. In practice the
  // property name would have been obtained from enumerating the properties in
  // JS code that looks something like this:
  //    for(key in hostObj) {
  //      if (key in hostObj)
  //        ...
  //    }
  return true;
}

static ExecutionStatus raiseErrorForOverridingStaticBuiltin(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    Handle<SymbolID> name) {
  Handle<StringPrimitive> methodNameHnd =
      runtime.makeHandle(runtime.getStringPrimFromSymbolID(name.get()));
  // If the 'name' property does not exist or is an accessor, we don't display
  // the name.
  NamedPropertyDescriptor desc;
  auto *obj = JSObject::getNamedDescriptorPredefined(
      selfHandle, runtime, Predefined::name, desc);
  assert(
      !selfHandle->isProxyObject() &&
      "raiseErrorForOverridingStaticBuiltin cannot be used with proxy objects");

  if (!obj || desc.flags.accessor) {
    return runtime.raiseTypeError(
        TwineChar16("Attempting to override read-only builtin method '") +
        TwineChar16(methodNameHnd.get()) + "'");
  }

  // Display the name property of the builtin object if it is a string.
  auto objNameRes =
      JSObject::getNamedSlotValue(createPseudoHandle(obj), runtime, desc);
  if (LLVM_UNLIKELY(objNameRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  PseudoHandle<StringPrimitive> objName =
      PseudoHandle<StringPrimitive>::dyn_vmcast(std::move(*objNameRes));
  if (!objName) {
    return runtime.raiseTypeError(
        TwineChar16("Attempting to override read-only builtin method '") +
        TwineChar16(methodNameHnd.get()) + "'");
  }

  return runtime.raiseTypeError(
      TwineChar16("Attempting to override read-only builtin method '") +
      TwineChar16(objName.get()) + "." + TwineChar16(methodNameHnd.get()) +
      "'");
}

CallResult<bool> JSObject::putNamedWithReceiver_RJS(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    SymbolID name,
    Handle<> valueHandle,
    Handle<> receiver,
    PropOpFlags opFlags) {
  NamedPropertyDescriptor desc;

  // Look for the property in this object or along the prototype chain.
  // `name` will not be freed before this function returns,
  // so it will outlive the lifetime of `desc`.
  JSObject *propObj = getNamedDescriptorUnsafe(
      selfHandle,
      runtime,
      name,
      PropertyFlags::defaultNewNamedPropertyFlags(),
      desc);

  // If the property exists (or, we hit a proxy/hostobject on the way
  // up the chain)
  if (propObj) {
    // Get the simple case out of the way: If the property already
    // exists on selfHandle, is not an accessor, selfHandle and
    // receiver are the same, selfHandle is not a host
    // object/proxy/internal setter, and the property is writable,
    // just write into the same slot.

    if (LLVM_LIKELY(
            *selfHandle == propObj &&
            selfHandle.getHermesValue().getRaw() == receiver->getRaw() &&
            !desc.flags.accessor && !desc.flags.internalSetter &&
            !desc.flags.hostObject && !desc.flags.proxyObject &&
            desc.flags.writable)) {
      auto shv = SmallHermesValue::encodeHermesValue(*valueHandle, runtime);
      setNamedSlotValueUnsafe(*selfHandle, runtime, desc, shv);
      return true;
    }

    if (LLVM_UNLIKELY(desc.flags.accessor)) {
      auto *accessor = vmcast<PropertyAccessor>(
          getNamedSlotValueUnsafe(propObj, runtime, desc).getObject(runtime));

      // If it is a read-only accessor, fail.
      if (!accessor->setter) {
        if (opFlags.getThrowOnError()) {
          return runtime.raiseTypeError(
              TwineChar16("Cannot assign to property '") +
              runtime.getIdentifierTable().getStringViewForDev(runtime, name) +
              "' which has only a getter");
        }
        return false;
      }

      // Execute the accessor on this object.
      if (accessor->setter.getNonNull(runtime)->executeCall1(
              runtime.makeHandle(accessor->setter),
              runtime,
              receiver,
              *valueHandle) == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      return true;
    }

    if (LLVM_UNLIKELY(desc.flags.proxyObject)) {
      assert(
          !opFlags.getMustExist() &&
          "MustExist cannot be used with Proxy objects");
      CallResult<bool> setRes = JSProxy::setNamed(
          runtime.makeHandle(propObj), runtime, name, valueHandle, receiver);
      if (LLVM_UNLIKELY(setRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      if (!*setRes && opFlags.getThrowOnError()) {
        return runtime.raiseTypeError(
            TwineChar16("Proxy set returned false for property '") +
            runtime.getIdentifierTable().getStringView(runtime, name) + "'");
      }
      return setRes;
    }

    if (LLVM_UNLIKELY(!desc.flags.writable)) {
      if (desc.flags.staticBuiltin) {
        return raiseErrorForOverridingStaticBuiltin(
            selfHandle, runtime, runtime.makeHandle(name));
      }
      if (opFlags.getThrowOnError()) {
        return runtime.raiseTypeError(
            TwineChar16("Cannot assign to read-only property '") +
            runtime.getIdentifierTable().getStringViewForDev(runtime, name) +
            "'");
      }
      return false;
    }

    if (*selfHandle == propObj && desc.flags.internalSetter) {
      return internalSetter(
          selfHandle, runtime, name, desc, valueHandle, opFlags);
    }
  }

  // The property does not exist as an conventional own property on
  // this object.

  MutableHandle<JSObject> receiverHandle{runtime, *selfHandle};
  MutableHandle<SymbolID> tmpSymbolStorage{runtime};
  if (selfHandle.getHermesValue().getRaw() != receiver->getRaw() ||
      receiverHandle->isHostObject() || receiverHandle->isProxyObject()) {
    if (selfHandle.getHermesValue().getRaw() != receiver->getRaw()) {
      receiverHandle = dyn_vmcast<JSObject>(*receiver);
    }
    if (!receiverHandle) {
      return false;
    }

    if (getOwnNamedDescriptor(receiverHandle, runtime, name, desc)) {
      if (LLVM_UNLIKELY(desc.flags.accessor || !desc.flags.writable)) {
        return false;
      }

      assert(
          !receiverHandle->isHostObject() && !receiverHandle->isProxyObject() &&
          "getOwnNamedDescriptor never sets hostObject or proxyObject flags");
      auto shv = SmallHermesValue::encodeHermesValue(*valueHandle, runtime);
      setNamedSlotValueUnsafe(*receiverHandle, runtime, desc, shv);
      return true;
    }

    // Now deal with host and proxy object cases.  We need to call
    // getOwnComputedPrimitiveDescriptor because it knows how to call
    // the [[getOwnProperty]] Proxy impl if needed.
    if (LLVM_UNLIKELY(
            receiverHandle->isHostObject() ||
            receiverHandle->isProxyObject())) {
      if (receiverHandle->isHostObject()) {
        return vmcast<HostObject>(receiverHandle.get())
            ->set(name, *valueHandle);
      }
      ComputedPropertyDescriptor desc;
      Handle<> nameValHandle = runtime.makeHandle(name);
      CallResult<bool> descDefinedRes = getOwnComputedPrimitiveDescriptor(
          receiverHandle,
          runtime,
          nameValHandle,
          IgnoreProxy::No,
          tmpSymbolStorage,
          desc);
      if (LLVM_UNLIKELY(descDefinedRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      DefinePropertyFlags dpf;
      if (*descDefinedRes) {
        dpf.setValue = 1;
      } else {
        dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
      }
      return JSProxy::defineOwnProperty(
          receiverHandle, runtime, nameValHandle, dpf, valueHandle, opFlags);
    }
  }

  // Does the caller require it to exist?
  if (LLVM_UNLIKELY(opFlags.getMustExist())) {
    return runtime.raiseReferenceError(
        TwineChar16("Property '") +
        runtime.getIdentifierTable().getStringViewForDev(runtime, name) +
        "' doesn't exist");
  }

  // Add a new property.

  return addOwnProperty(
      receiverHandle,
      runtime,
      name,
      DefinePropertyFlags::getDefaultNewPropertyFlags(),
      valueHandle,
      opFlags);
}

CallResult<bool> JSObject::putNamedOrIndexed(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    SymbolID name,
    Handle<> valueHandle,
    PropOpFlags opFlags) {
  if (LLVM_UNLIKELY(selfHandle->flags_.indexedStorage)) {
    // Note that getStringView can be satisfied without materializing the
    // Identifier.
    const auto strView =
        runtime.getIdentifierTable().getStringView(runtime, name);
    if (auto nameAsIndex = toArrayIndex(strView)) {
      return putComputed_RJS(
          selfHandle,
          runtime,
          runtime.makeHandle(HermesValue::encodeNumberValue(*nameAsIndex)),
          valueHandle,
          opFlags);
    }
    // Here we have indexed properties but the symbol was not index-like.
    // Fall through to putNamed().
  }
  return putNamed_RJS(selfHandle, runtime, name, valueHandle, opFlags);
}

CallResult<bool> JSObject::putComputedWithReceiver_RJS(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    Handle<> nameValHandle,
    Handle<> valueHandle,
    Handle<> receiver,
    PropOpFlags opFlags) {
  assert(
      !opFlags.getMustExist() &&
      "mustExist flag cannot be used with computed properties");

  // Try the fast-path first: has "index-like" properties, the "name"
  // already is a valid integer index, selfHandle and receiver are the
  // same, and it is present in storage.
  if (selfHandle->flags_.fastIndexProperties) {
    if (auto arrayIndex = toArrayIndexFastPath(*nameValHandle)) {
      if (selfHandle.getHermesValue().getRaw() == receiver->getRaw()) {
        if (haveOwnIndexed(selfHandle.get(), runtime, *arrayIndex)) {
          auto result =
              setOwnIndexed(selfHandle, runtime, *arrayIndex, valueHandle);
          if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION))
            return ExecutionStatus::EXCEPTION;
          if (LLVM_LIKELY(*result))
            return true;
          if (opFlags.getThrowOnError()) {
            // TODO: better message.
            return runtime.raiseTypeError(
                "Cannot assign to read-only property");
          }
          return false;
        }
      }
    }
  }

  // If nameValHandle is an object, we should convert it to string now,
  // because toString may have side-effect, and we want to do this only
  // once.
  auto converted = toPropertyKeyIfObject(runtime, nameValHandle);
  if (LLVM_UNLIKELY(converted == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto nameValPrimitiveHandle = *converted;

  ComputedPropertyDescriptor desc;

  // Look for the property in this object or along the prototype chain.
  MutableHandle<JSObject> propObj{runtime};
  MutableHandle<SymbolID> tmpSymbolStorage{runtime};
  if (LLVM_UNLIKELY(
          getComputedPrimitiveDescriptor(
              selfHandle,
              runtime,
              nameValPrimitiveHandle,
              propObj,
              tmpSymbolStorage,
              desc) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // If the property exists (or, we hit a proxy/hostobject on the way
  // up the chain)
  if (propObj) {
    // Get the simple case out of the way: If the property already
    // exists on selfHandle, is not an accessor, selfHandle and
    // receiver are the same, selfHandle is not a host
    // object/proxy/internal setter, and the property is writable,
    // just write into the same slot.

    if (LLVM_LIKELY(
            selfHandle == propObj &&
            selfHandle.getHermesValue().getRaw() == receiver->getRaw() &&
            !desc.flags.accessor && !desc.flags.internalSetter &&
            !desc.flags.hostObject && !desc.flags.proxyObject &&
            desc.flags.writable)) {
      if (LLVM_UNLIKELY(
              setComputedSlotValueUnsafe(
                  selfHandle, runtime, desc, valueHandle) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      return true;
    }

    // Is it an accessor?
    if (LLVM_UNLIKELY(desc.flags.accessor)) {
      auto *accessor = vmcast<PropertyAccessor>(
          getComputedSlotValueUnsafe(propObj, runtime, desc));

      // If it is a read-only accessor, fail.
      if (!accessor->setter) {
        if (opFlags.getThrowOnError()) {
          return runtime.raiseTypeErrorForValue(
              "Cannot assign to property ",
              nameValPrimitiveHandle,
              " which has only a getter");
        }
        return false;
      }

      // Execute the accessor on this object.
      if (accessor->setter.getNonNull(runtime)->executeCall1(
              runtime.makeHandle(accessor->setter),
              runtime,
              receiver,
              valueHandle.get()) == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      return true;
    }

    if (LLVM_UNLIKELY(desc.flags.proxyObject)) {
      assert(
          !opFlags.getMustExist() &&
          "MustExist cannot be used with Proxy objects");
      CallResult<Handle<>> key = toPropertyKey(runtime, nameValPrimitiveHandle);
      if (key == ExecutionStatus::EXCEPTION)
        return ExecutionStatus::EXCEPTION;
      CallResult<bool> setRes =
          JSProxy::setComputed(propObj, runtime, *key, valueHandle, receiver);
      if (LLVM_UNLIKELY(setRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      if (!*setRes && opFlags.getThrowOnError()) {
        // TODO: better message.
        return runtime.raiseTypeError(
            TwineChar16("Proxy trap returned false for property"));
      }
      return setRes;
    }

    if (LLVM_UNLIKELY(!desc.flags.writable)) {
      if (desc.flags.staticBuiltin) {
        SymbolID id{};
        LAZY_TO_IDENTIFIER(runtime, nameValPrimitiveHandle, id);
        return raiseErrorForOverridingStaticBuiltin(
            selfHandle, runtime, runtime.makeHandle(id));
      }
      if (opFlags.getThrowOnError()) {
        return runtime.raiseTypeErrorForValue(
            "Cannot assign to read-only property ", nameValPrimitiveHandle, "");
      }
      return false;
    }

    if (selfHandle == propObj && desc.flags.internalSetter) {
      SymbolID id{};
      LAZY_TO_IDENTIFIER(runtime, nameValPrimitiveHandle, id);
      return internalSetter(
          selfHandle,
          runtime,
          id,
          desc.castToNamedPropertyDescriptorRef(),
          valueHandle,
          opFlags);
    }
  }

  // The property does not exist as an conventional own property on
  // this object.

  MutableHandle<JSObject> receiverHandle{runtime, *selfHandle};
  if (selfHandle.getHermesValue().getRaw() != receiver->getRaw() ||
      receiverHandle->isHostObject() || receiverHandle->isProxyObject()) {
    if (selfHandle.getHermesValue().getRaw() != receiver->getRaw()) {
      receiverHandle = dyn_vmcast<JSObject>(*receiver);
    }
    if (!receiverHandle) {
      return false;
    }
    ComputedPropertyDescriptor existingDesc;
    CallResult<bool> descDefinedRes = getOwnComputedPrimitiveDescriptor(
        receiverHandle,
        runtime,
        nameValPrimitiveHandle,
        IgnoreProxy::No,
        tmpSymbolStorage,
        existingDesc);
    if (LLVM_UNLIKELY(descDefinedRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    DefinePropertyFlags dpf;
    if (*descDefinedRes) {
      if (LLVM_UNLIKELY(
              existingDesc.flags.accessor || !existingDesc.flags.writable)) {
        return false;
      }

      if (LLVM_LIKELY(
              !existingDesc.flags.internalSetter &&
              !receiverHandle->isHostObject() &&
              !receiverHandle->isProxyObject())) {
        if (LLVM_UNLIKELY(
                setComputedSlotValueUnsafe(
                    receiverHandle, runtime, existingDesc, valueHandle) ==
                ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
        return true;
      }
    }

    // At this point, either the descriptor exists on the receiver,
    // but it's a corner case; or, there was no descriptor.
    if (LLVM_UNLIKELY(
            existingDesc.flags.internalSetter ||
            receiverHandle->isHostObject() ||
            receiverHandle->isProxyObject())) {
      // If putComputed is called on a proxy whose target's prototype
      // is an array with a propname of 'length', then internalSetter
      // will be true, and the receiver will be a proxy.  In that case,
      // proxy wins.
      if (receiverHandle->isProxyObject()) {
        if (*descDefinedRes) {
          dpf.setValue = 1;
        } else {
          dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
        }
        return JSProxy::defineOwnProperty(
            receiverHandle,
            runtime,
            nameValPrimitiveHandle,
            dpf,
            valueHandle,
            opFlags);
      }
      SymbolID id{};
      LAZY_TO_IDENTIFIER(runtime, nameValPrimitiveHandle, id);
      if (existingDesc.flags.internalSetter) {
        return internalSetter(
            receiverHandle,
            runtime,
            id,
            existingDesc.castToNamedPropertyDescriptorRef(),
            valueHandle,
            opFlags);
      }
      assert(
          receiverHandle->isHostObject() && "descriptor flags are impossible");
      return vmcast<HostObject>(receiverHandle.get())->set(id, *valueHandle);
    }
  }

  /// Can we add more properties?
  if (LLVM_UNLIKELY(!receiverHandle->isExtensible())) {
    if (opFlags.getThrowOnError()) {
      return runtime.raiseTypeError(
          "cannot add a new property"); // TODO: better message.
    }
    return false;
  }

  // If we have indexed storage we must check whether the property is an index,
  // and if it is, store it in indexed storage.
  if (receiverHandle->flags_.indexedStorage) {
    OptValue<uint32_t> arrayIndex;
    MutableHandle<StringPrimitive> strPrim{runtime};
    TO_ARRAY_INDEX(runtime, nameValPrimitiveHandle, strPrim, arrayIndex);
    if (arrayIndex) {
      // Check whether we need to update array's ".length" property.
      if (auto *array = dyn_vmcast<JSArray>(receiverHandle.get())) {
        if (LLVM_UNLIKELY(*arrayIndex >= JSArray::getLength(array, runtime))) {
          auto cr = putNamed_RJS(
              receiverHandle,
              runtime,
              Predefined::getSymbolID(Predefined::length),
              runtime.makeHandle(
                  HermesValue::encodeNumberValue(*arrayIndex + 1)),
              opFlags);
          if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION))
            return ExecutionStatus::EXCEPTION;
          if (LLVM_UNLIKELY(!*cr))
            return false;
        }
      }

      auto result =
          setOwnIndexed(receiverHandle, runtime, *arrayIndex, valueHandle);
      if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION))
        return ExecutionStatus::EXCEPTION;
      if (LLVM_LIKELY(*result))
        return true;

      if (opFlags.getThrowOnError()) {
        // TODO: better message.
        return runtime.raiseTypeError("Cannot assign to read-only property");
      }
      return false;
    }
  }

  SymbolID id{};
  LAZY_TO_IDENTIFIER(runtime, nameValPrimitiveHandle, id);

  // Add a new named property.
  return addOwnProperty(
      receiverHandle,
      runtime,
      id,
      DefinePropertyFlags::getDefaultNewPropertyFlags(),
      valueHandle,
      opFlags);
}

CallResult<bool> JSObject::deleteNamed(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    SymbolID name,
    PropOpFlags opFlags) {
  assert(
      !opFlags.getMustExist() && "mustExist cannot be specified when deleting");

  // Find the property by name.
  NamedPropertyDescriptor desc;
  auto pos = findProperty(selfHandle, runtime, name, desc);

  // If the property doesn't exist in this object, return success.
  if (!pos) {
    if (LLVM_LIKELY(
            !selfHandle->flags_.lazyObject &&
            !selfHandle->flags_.proxyObject)) {
      return true;
    } else if (selfHandle->flags_.lazyObject) {
      // object is lazy, initialize and read again.
      initializeLazyObject(runtime, selfHandle);
      pos = findProperty(selfHandle, runtime, name, desc);
      if (!pos) // still not there, return true.
        return true;
    } else {
      assert(selfHandle->flags_.proxyObject && "object flags are impossible");
      return proxyOpFlags(
          runtime,
          opFlags,
          "Proxy delete returned false",
          JSProxy::deleteNamed(selfHandle, runtime, name));
    }
  }
  // If the property isn't configurable, fail.
  if (LLVM_UNLIKELY(!desc.flags.configurable)) {
    if (opFlags.getThrowOnError()) {
      return runtime.raiseTypeError(
          TwineChar16("Property '") +
          runtime.getIdentifierTable().getStringViewForDev(runtime, name) +
          "' is not configurable");
    }
    return false;
  }

  // Clear the deleted property value to prevent memory leaks.
  setNamedSlotValueUnsafe(
      *selfHandle, runtime, desc, SmallHermesValue::encodeEmptyValue());

  // Perform the actual deletion.
  auto newClazz = HiddenClass::deleteProperty(
      runtime.makeHandle(selfHandle->clazz_), runtime, *pos);
  selfHandle->clazz_.setNonNull(runtime, *newClazz, runtime.getHeap());

  return true;
}

CallResult<bool> JSObject::deleteComputed(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    Handle<> nameValHandle,
    PropOpFlags opFlags) {
  assert(
      !opFlags.getMustExist() && "mustExist cannot be specified when deleting");

  // If nameValHandle is an object, we should convert it to string now,
  // because toString may have side-effect, and we want to do this only
  // once.
  auto converted = toPropertyKeyIfObject(runtime, nameValHandle);
  if (LLVM_UNLIKELY(converted == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  auto nameValPrimitiveHandle = *converted;

  // If the name is a valid integer array index, store it here.
  OptValue<uint32_t> arrayIndex;

  // If we have indexed storage, we must attempt to convert the name to array
  // index, even if the conversion is expensive.
  if (selfHandle->flags_.indexedStorage) {
    MutableHandle<StringPrimitive> strPrim{runtime};
    TO_ARRAY_INDEX(runtime, nameValPrimitiveHandle, strPrim, arrayIndex);
  }

  // Try the fast-path first: the "name" is a valid array index and we don't
  // have "index-like" named properties.
  if (arrayIndex && selfHandle->flags_.fastIndexProperties) {
    // Delete the indexed property.
    if (deleteOwnIndexed(selfHandle, runtime, *arrayIndex))
      return true;

    // Cannot delete property (for example this may be a typed array).
    if (opFlags.getThrowOnError()) {
      // TODO: better error message.
      return runtime.raiseTypeError("Cannot delete property");
    }
    return false;
  }

  // slow path, check if object is lazy before continuing.
  if (LLVM_UNLIKELY(selfHandle->flags_.lazyObject)) {
    // initialize and try again.
    initializeLazyObject(runtime, selfHandle);
    return deleteComputed(selfHandle, runtime, nameValHandle, opFlags);
  }

  // Convert the string to an SymbolID;
  SymbolID id;
  LAZY_TO_IDENTIFIER(runtime, nameValPrimitiveHandle, id);

  // Find the property by name.
  NamedPropertyDescriptor desc;
  auto pos = findProperty(selfHandle, runtime, id, desc);

  // If the property exists, make sure it is configurable.
  if (pos) {
    // If the property isn't configurable, fail.
    if (LLVM_UNLIKELY(!desc.flags.configurable)) {
      if (opFlags.getThrowOnError()) {
        // TODO: a better message.
        return runtime.raiseTypeError("Property is not configurable");
      }
      return false;
    }
  }

  // At this point we know that the named property either doesn't exist, or
  // is configurable and so can be deleted, or the object is a Proxy.

  // If it is an "index-like" property, we must also delete the "shadow" indexed
  // property in order to keep Array.length correct.
  if (arrayIndex) {
    if (!deleteOwnIndexed(selfHandle, runtime, *arrayIndex)) {
      // Cannot delete property (for example this may be a typed array).
      if (opFlags.getThrowOnError()) {
        // TODO: better error message.
        return runtime.raiseTypeError("Cannot delete property");
      }
      return false;
    }
  }

  if (pos) {
    // delete the named property (if it exists).
    // Clear the deleted property value to prevent memory leaks.
    setNamedSlotValueUnsafe(
        *selfHandle, runtime, desc, SmallHermesValue::encodeEmptyValue());

    // Remove the property descriptor.
    auto newClazz = HiddenClass::deleteProperty(
        runtime.makeHandle(selfHandle->clazz_), runtime, *pos);
    selfHandle->clazz_.setNonNull(runtime, *newClazz, runtime.getHeap());
  } else if (LLVM_UNLIKELY(selfHandle->flags_.proxyObject)) {
    CallResult<Handle<>> key = toPropertyKey(runtime, nameValPrimitiveHandle);
    if (key == ExecutionStatus::EXCEPTION)
      return ExecutionStatus::EXCEPTION;
    return proxyOpFlags(
        runtime,
        opFlags,
        "Proxy delete returned false",
        JSProxy::deleteComputed(selfHandle, runtime, *key));
  }

  return true;
}

CallResult<bool> JSObject::defineOwnPropertyInternal(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    SymbolID name,
    DefinePropertyFlags dpFlags,
    Handle<> valueOrAccessor,
    PropOpFlags opFlags) {
  assert(
      !opFlags.getMustExist() && "cannot use mustExist with defineOwnProperty");
  assert(
      !(dpFlags.setValue && dpFlags.isAccessor()) &&
      "Cannot set both value and accessor");
  assert(
      (dpFlags.setValue || dpFlags.isAccessor() ||
       valueOrAccessor.get().isUndefined()) &&
      "value must be undefined when all of setValue/setSetter/setGetter are "
      "false");
#ifndef NDEBUG
  if (dpFlags.isAccessor()) {
    assert(valueOrAccessor.get().isPointer() && "accessor must be non-empty");
    assert(
        !dpFlags.setWritable && !dpFlags.writable &&
        "writable must not be set with accessors");
  }
#endif

  // Is it an existing property.
  NamedPropertyDescriptor desc;
  auto pos = findProperty(selfHandle, runtime, name, desc);
  if (pos) {
    return updateOwnProperty(
        selfHandle,
        runtime,
        name,
        *pos,
        desc,
        dpFlags,
        valueOrAccessor,
        opFlags);
  }

  if (LLVM_UNLIKELY(
          selfHandle->flags_.lazyObject || selfHandle->flags_.proxyObject)) {
    if (selfHandle->flags_.proxyObject) {
      return JSProxy::defineOwnProperty(
          selfHandle,
          runtime,
          name.isUniqued() ? runtime.makeHandle(HermesValue::encodeStringValue(
                                 runtime.getStringPrimFromSymbolID(name)))
                           : runtime.makeHandle(name),
          dpFlags,
          valueOrAccessor,
          opFlags);
    }
    assert(selfHandle->flags_.lazyObject && "descriptor flags are impossible");
    // if the property was not found and the object is lazy we need to
    // initialize it and try again.
    JSObject::initializeLazyObject(runtime, selfHandle);
    return defineOwnPropertyInternal(
        selfHandle, runtime, name, dpFlags, valueOrAccessor, opFlags);
  }

  return addOwnProperty(
      selfHandle, runtime, name, dpFlags, valueOrAccessor, opFlags);
}

ExecutionStatus JSObject::defineNewOwnProperty(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    SymbolID name,
    PropertyFlags propertyFlags,
    Handle<> valueOrAccessor) {
  assert(
      !selfHandle->flags_.proxyObject &&
      "definedNewOwnProperty cannot be used with proxy objects");
  assert(
      !(propertyFlags.accessor && !valueOrAccessor.get().isPointer()) &&
      "accessor must be non-empty");
  assert(
      !(propertyFlags.accessor && propertyFlags.writable) &&
      "writable must not be set with accessors");
  assert(
      !HiddenClass::debugIsPropertyDefined(
          selfHandle->clazz_.get(runtime), runtime, name) &&
      "new property is already defined");

  return addOwnPropertyImpl(
      selfHandle, runtime, name, propertyFlags, valueOrAccessor);
}

CallResult<bool> JSObject::defineOwnComputedPrimitive(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    Handle<> nameValHandle,
    DefinePropertyFlags dpFlags,
    Handle<> valueOrAccessor,
    PropOpFlags opFlags) {
  assert(
      !nameValHandle->isObject() &&
      "nameValHandle passed to "
      "defineOwnComputedPrimitive() cannot be "
      "an object");
  assert(
      !opFlags.getMustExist() && "cannot use mustExist with defineOwnProperty");
  assert(
      !(dpFlags.setValue && dpFlags.isAccessor()) &&
      "Cannot set both value and accessor");
  assert(
      (dpFlags.setValue || dpFlags.isAccessor() ||
       valueOrAccessor.get().isUndefined()) &&
      "value must be undefined when all of setValue/setSetter/setGetter are "
      "false");
  assert(
      !dpFlags.enableInternalSetter &&
      "Cannot set internalSetter on a computed property");
#ifndef NDEBUG
  if (dpFlags.isAccessor()) {
    assert(valueOrAccessor.get().isPointer() && "accessor must be non-empty");
    assert(
        !dpFlags.setWritable && !dpFlags.writable &&
        "writable must not be set with accessors");
  }
#endif

  // If the name is a valid integer array index, store it here.
  OptValue<uint32_t> arrayIndex;

  // If we have indexed storage, we must attempt to convert the name to array
  // index, even if the conversion is expensive.
  if (selfHandle->flags_.indexedStorage) {
    MutableHandle<StringPrimitive> strPrim{runtime};
    TO_ARRAY_INDEX(runtime, nameValHandle, strPrim, arrayIndex);
  }

  SymbolID id{};

  // If not storing a property with an array index name, or if we don't have
  // indexed storage, just pass to the named routine.
  if (!arrayIndex) {
    // TODO(T125334872): properly handle the case when self is a TypedArray.
    LAZY_TO_IDENTIFIER(runtime, nameValHandle, id);
    return defineOwnPropertyInternal(
        selfHandle, runtime, id, dpFlags, valueOrAccessor, opFlags);
  }

  // At this point we know that we have indexed storage and that the property
  // has an index-like name.

  // First check if a named property with the same name exists.
  if (selfHandle->clazz_.getNonNull(runtime)->getHasIndexLikeProperties()) {
    LAZY_TO_IDENTIFIER(runtime, nameValHandle, id);

    NamedPropertyDescriptor desc;
    auto pos = findProperty(selfHandle, runtime, id, desc);
    // If we found a named property, update it.
    if (pos) {
      return updateOwnProperty(
          selfHandle,
          runtime,
          id,
          *pos,
          desc,
          dpFlags,
          valueOrAccessor,
          opFlags);
    }
  }

  // Does an indexed property with that index exist?
  auto indexedPropPresent =
      getOwnIndexedPropertyFlags(selfHandle.get(), runtime, *arrayIndex);
  if (indexedPropPresent) {
    // The current value of the property.
    Handle<> curValueOrAccessor =
        runtime.makeHandle(getOwnIndexed(selfHandle, runtime, *arrayIndex));

    auto updateStatus = checkPropertyUpdate(
        runtime,
        *indexedPropPresent,
        dpFlags,
        *curValueOrAccessor,
        valueOrAccessor,
        opFlags);
    if (updateStatus == ExecutionStatus::EXCEPTION)
      return ExecutionStatus::EXCEPTION;
    if (updateStatus->first == PropertyUpdateStatus::failed)
      return false;

    // The property update is valid, but can the property remain an "indexed"
    // property, or do we need to convert it to a named property?
    // If the property flags didn't change, the property remains indexed.
    if (updateStatus->second == *indexedPropPresent) {
      // If the value doesn't change, we are done.
      if (updateStatus->first == PropertyUpdateStatus::done)
        return true;

      // If we successfully updated the value, we are done.
      auto result =
          setOwnIndexed(selfHandle, runtime, *arrayIndex, valueOrAccessor);
      if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION))
        return ExecutionStatus::EXCEPTION;
      if (*result)
        return true;

      if (opFlags.getThrowOnError()) {
        // TODO: better error message.
        return runtime.raiseTypeError("cannot change read-only property value");
      }

      return false;
    }

    // OK, we need to convert an indexed property to a named one.

    // Check whether to use the supplied value, or to reuse the old one, as we
    // are simply reconfiguring it.
    MutableHandle<> value{runtime};
    if (dpFlags.setValue || dpFlags.isAccessor()) {
      value = valueOrAccessor.get();
    } else {
      value = *curValueOrAccessor;
    }

    // Update dpFlags to match the existing property flags.
    dpFlags.setEnumerable = 1;
    dpFlags.setWritable = 1;
    dpFlags.setConfigurable = 1;
    dpFlags.enumerable = updateStatus->second.enumerable;
    dpFlags.writable = updateStatus->second.writable;
    dpFlags.configurable = updateStatus->second.configurable;

    // Delete the existing indexed property.
    if (!deleteOwnIndexed(selfHandle, runtime, *arrayIndex)) {
      if (opFlags.getThrowOnError()) {
        // TODO: better error message.
        return runtime.raiseTypeError("Cannot define property");
      }
      return false;
    }

    // Add the new named property.
    LAZY_TO_IDENTIFIER(runtime, nameValHandle, id);
    return addOwnProperty(selfHandle, runtime, id, dpFlags, value, opFlags);
  }

  /// Can we add new properties?
  if (!selfHandle->isExtensible()) {
    if (opFlags.getThrowOnError()) {
      return runtime.raiseTypeError(
          "cannot add a new property"); // TODO: better message.
    }
    return false;
  }

  // This is a new property with an index-like name.
  // Check whether we need to update array's ".length" property.
  bool updateLength = false;
  if (auto arrayHandle = Handle<JSArray>::dyn_vmcast(selfHandle)) {
    if (LLVM_UNLIKELY(
            *arrayIndex >= JSArray::getLength(*arrayHandle, runtime))) {
      NamedPropertyDescriptor lengthDesc;
      bool lengthPresent = getOwnNamedDescriptor(
          arrayHandle,
          runtime,
          Predefined::getSymbolID(Predefined::length),
          lengthDesc);
      (void)lengthPresent;
      assert(lengthPresent && ".length must be present in JSArray");

      if (!lengthDesc.flags.writable) {
        if (opFlags.getThrowOnError()) {
          return runtime.raiseTypeError(
              "Cannot assign to read-only 'length' property of array");
        }
        return false;
      }

      updateLength = true;
    }
  }

  bool newIsIndexed = canNewPropertyBeIndexed(dpFlags);
  if (newIsIndexed) {
    auto result = setOwnIndexed(
        selfHandle,
        runtime,
        *arrayIndex,
        dpFlags.setValue ? valueOrAccessor : Runtime::getUndefinedValue());
    if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION))
      return ExecutionStatus::EXCEPTION;
    if (!*result) {
      if (opFlags.getThrowOnError()) {
        // TODO: better error message.
        return runtime.raiseTypeError("Cannot define property");
      }
      return false;
    }
  }

  // If this is an array and we need to update ".length", do so.
  if (updateLength) {
    // This should always succeed since we are simply enlarging the length.
    auto res = JSArray::setLength(
        Handle<JSArray>::vmcast(selfHandle), runtime, *arrayIndex + 1, opFlags);
    (void)res;
    assert(
        res != ExecutionStatus::EXCEPTION && *res &&
        "JSArray::setLength() failed unexpectedly");
  }

  if (newIsIndexed)
    return true;

  // We are adding a new property with an index-like name.
  // TODO(T125334872): properly handle the case when self is a TypedArray.
  LAZY_TO_IDENTIFIER(runtime, nameValHandle, id);
  return addOwnProperty(
      selfHandle, runtime, id, dpFlags, valueOrAccessor, opFlags);
}

CallResult<bool> JSObject::defineOwnComputed(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    Handle<> nameValHandle,
    DefinePropertyFlags dpFlags,
    Handle<> valueOrAccessor,
    PropOpFlags opFlags) {
  auto converted = toPropertyKeyIfObject(runtime, nameValHandle);
  if (LLVM_UNLIKELY(converted == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  return defineOwnComputedPrimitive(
      selfHandle, runtime, *converted, dpFlags, valueOrAccessor, opFlags);
}

std::string JSObject::getNameIfExists(PointerBase &base) {
  // Try "displayName" first, if it is defined.
  if (auto nameVal = tryGetNamedNoAlloc(
          this, base, Predefined::getSymbolID(Predefined::displayName))) {
    if (auto *name = dyn_vmcast<StringPrimitive>(nameVal->unboxToHV(base))) {
      return converter(name);
    }
  }
  // Next, use "name" if it is defined.
  if (auto nameVal = tryGetNamedNoAlloc(
          this, base, Predefined::getSymbolID(Predefined::name))) {
    if (auto *name = dyn_vmcast<StringPrimitive>(nameVal->unboxToHV(base))) {
      return converter(name);
    }
  }
  // There is no other way to access the "name" property on an object.
  return "";
}

#ifdef HERMES_MEMORY_INSTRUMENTATION
std::string JSObject::getHeuristicTypeName(GC &gc) {
  PointerBase &base = gc.getPointerBase();
  if (auto constructorVal = tryGetNamedNoAlloc(
          this, base, Predefined::getSymbolID(Predefined::constructor))) {
    if (auto *constructor = dyn_vmcast<JSObject>(
            constructorVal->unboxToHV(gc.getPointerBase()))) {
      auto name = constructor->getNameIfExists(base);
      // If the constructor's name doesn't exist, or it is just the object
      // constructor, attempt to find a different name.
      if (!name.empty() && name != "Object")
        return name;
    }
  }

  std::string name = getVT()->snapshotMetaData.defaultNameForNode(this);
  // A constructor's name was not found, check if the object is in dictionary
  // mode.
  if (getClass(base)->isDictionary()) {
    return name + "(Dictionary)";
  }

  // If it's not an Object, the CellKind is most likely good enough on its own
  if (getKind() != CellKind::JSObjectKind) {
    return name;
  }

  // If the object isn't a dictionary, and it has only a few property names,
  // make the name based on those property names.
  std::vector<std::string> propertyNames;
  HiddenClass::forEachPropertyNoAlloc(
      getClass(base),
      base,
      [&gc, &propertyNames](SymbolID id, NamedPropertyDescriptor) {
        if (InternalProperty::isInternal(id)) {
          // Internal properties aren't user-visible, skip them.
          return;
        }
        propertyNames.emplace_back(gc.convertSymbolToUTF8(id));
      });
  // NOTE: One option is to sort the property names before truncation, to
  // reduce the number of groups; however, by not sorting them it makes it
  // easier to spot sets of objects with the same properties but in different
  // orders, and thus find HiddenClass optimizations to make.

  // For objects with a lot of properties but aren't in dictionary mode yet,
  // keep the number displayed small.
  constexpr int kMaxPropertiesForTypeName = 5;
  bool truncated = false;
  if (propertyNames.size() > kMaxPropertiesForTypeName) {
    propertyNames.erase(
        propertyNames.begin() + kMaxPropertiesForTypeName, propertyNames.end());
    truncated = true;
  }
  // The final name should look like Object(a, b, c).
  if (propertyNames.empty()) {
    // Don't add parentheses for objects with no properties.
    return name;
  }
  name += "(";
  bool first = true;
  for (const auto &prop : propertyNames) {
    if (!first) {
      name += ", ";
    }
    first = false;
    name += prop;
  }
  if (truncated) {
    // No need to check for comma edge case because this only happens for
    // greater than one property.
    static_assert(
        kMaxPropertiesForTypeName >= 1,
        "Property truncation should not happen for 0 properties");
    name += ", ...";
  }
  name += ")";
  return name;
}

std::string JSObject::_snapshotNameImpl(GCCell *cell, GC &gc) {
  auto *const self = vmcast<JSObject>(cell);
  return self->getHeuristicTypeName(gc);
}

void JSObject::_snapshotAddEdgesImpl(GCCell *cell, GC &gc, HeapSnapshot &snap) {
  auto *const self = vmcast<JSObject>(cell);

  // Add the prototype as a property edge, so it's easy for JS developers to
  // walk the prototype chain on their own.
  if (self->parent_) {
    snap.addNamedEdge(
        HeapSnapshot::EdgeType::Property,
        // __proto__ chosen for similarity to V8.
        "__proto__",
        gc.getObjectID(self->parent_));
  }

  HiddenClass::forEachPropertyNoAlloc(
      self->clazz_.get(gc.getPointerBase()),
      gc.getPointerBase(),
      [self, &gc, &snap](SymbolID id, NamedPropertyDescriptor desc) {
        if (InternalProperty::isInternal(id)) {
          // Internal properties aren't user-visible, skip them.
          return;
        }
        // Else, it's a user-visible property.
        HermesValue prop =
            getNamedSlotValueUnsafe(self, gc.getPointerBase(), desc.slot)
                .unboxToHV(gc.getPointerBase());
        const llvh::Optional<HeapSnapshot::NodeID> idForProp =
            gc.getSnapshotID(prop);
        if (!idForProp) {
          return;
        }
        std::string propName = gc.convertSymbolToUTF8(id);
        // If the property name is a valid array index, display it as an
        // "element" instead of a "property". This will put square brackets
        // around the number and sort it numerically rather than
        // alphabetically.
        if (auto index = ::hermes::toArrayIndex(propName)) {
          snap.addIndexedEdge(
              HeapSnapshot::EdgeType::Element,
              index.getValue(),
              idForProp.getValue());
        } else {
          snap.addNamedEdge(
              HeapSnapshot::EdgeType::Property, propName, idForProp.getValue());
        }
      });
}

void JSObject::_snapshotAddLocationsImpl(
    GCCell *cell,
    GC &gc,
    HeapSnapshot &snap) {
  auto *const self = vmcast<JSObject>(cell);
  PointerBase &base = gc.getPointerBase();
  // Add the location of the constructor function for this object, if that
  // constructor is a user-defined JS function.
  if (auto constructorVal = tryGetNamedNoAlloc(
          self, base, Predefined::getSymbolID(Predefined::constructor))) {
    if (constructorVal->isObject()) {
      if (auto *constructor =
              dyn_vmcast<JSFunction>(constructorVal->getObject(base))) {
        constructor->addLocationToSnapshot(snap, gc.getObjectID(self), gc);
      }
    }
  }
}
#endif

std::pair<uint32_t, uint32_t> JSObject::_getOwnIndexedRangeImpl(
    JSObject *self,
    Runtime &runtime) {
  return {0, 0};
}

bool JSObject::_haveOwnIndexedImpl(JSObject *self, Runtime &, uint32_t) {
  return false;
}

OptValue<PropertyFlags> JSObject::_getOwnIndexedPropertyFlagsImpl(
    JSObject *self,
    Runtime &runtime,
    uint32_t) {
  return llvh::None;
}

HermesValue
JSObject::_getOwnIndexedImpl(PseudoHandle<JSObject>, Runtime &, uint32_t) {
  return HermesValue::encodeEmptyValue();
}

CallResult<bool>
JSObject::_setOwnIndexedImpl(Handle<JSObject>, Runtime &, uint32_t, Handle<>) {
  return false;
}

bool JSObject::_deleteOwnIndexedImpl(Handle<JSObject>, Runtime &, uint32_t) {
  return false;
}

bool JSObject::_checkAllOwnIndexedImpl(
    JSObject * /*self*/,
    Runtime & /*runtime*/,
    ObjectVTable::CheckAllOwnIndexedMode /*mode*/) {
  return true;
}

void JSObject::preventExtensions(JSObject *self) {
  assert(
      !self->flags_.proxyObject &&
      "[[Extensible]] slot cannot be set directly on Proxy objects");
  self->flags_.noExtend = true;
}

CallResult<bool> JSObject::preventExtensions(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    PropOpFlags opFlags) {
  if (LLVM_UNLIKELY(selfHandle->isProxyObject())) {
    return JSProxy::preventExtensions(selfHandle, runtime, opFlags);
  }
  JSObject::preventExtensions(*selfHandle);
  return true;
}

ExecutionStatus JSObject::seal(Handle<JSObject> selfHandle, Runtime &runtime) {
  CallResult<bool> statusRes = JSObject::preventExtensions(
      selfHandle, runtime, PropOpFlags().plusThrowOnError());
  if (LLVM_UNLIKELY(statusRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  assert(
      *statusRes && "seal preventExtensions with ThrowOnError returned false");

  // Already sealed?
  if (selfHandle->flags_.sealed)
    return ExecutionStatus::RETURNED;

  auto newClazz = HiddenClass::makeAllNonConfigurable(
      runtime.makeHandle(selfHandle->clazz_), runtime);
  selfHandle->clazz_.setNonNull(runtime, *newClazz, runtime.getHeap());

  selfHandle->flags_.sealed = true;

  return ExecutionStatus::RETURNED;
}

ExecutionStatus JSObject::freeze(
    Handle<JSObject> selfHandle,
    Runtime &runtime) {
  CallResult<bool> statusRes = JSObject::preventExtensions(
      selfHandle, runtime, PropOpFlags().plusThrowOnError());
  if (LLVM_UNLIKELY(statusRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  assert(
      *statusRes &&
      "freeze preventExtensions with ThrowOnError returned false");

  // Already frozen?
  if (selfHandle->flags_.frozen)
    return ExecutionStatus::RETURNED;

  auto newClazz = HiddenClass::makeAllReadOnly(
      runtime.makeHandle(selfHandle->clazz_), runtime);
  selfHandle->clazz_.setNonNull(runtime, *newClazz, runtime.getHeap());

  selfHandle->flags_.frozen = true;
  selfHandle->flags_.sealed = true;

  return ExecutionStatus::RETURNED;
}

void JSObject::updatePropertyFlagsWithoutTransitions(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    PropertyFlags flagsToClear,
    PropertyFlags flagsToSet,
    OptValue<llvh::ArrayRef<SymbolID>> props) {
  auto newClazz = HiddenClass::updatePropertyFlagsWithoutTransitions(
      runtime.makeHandle(selfHandle->clazz_),
      runtime,
      flagsToClear,
      flagsToSet,
      props);
  selfHandle->clazz_.setNonNull(runtime, *newClazz, runtime.getHeap());
}

CallResult<bool> JSObject::isExtensible(
    PseudoHandle<JSObject> self,
    Runtime &runtime) {
  if (LLVM_UNLIKELY(self->isProxyObject())) {
    return JSProxy::isExtensible(runtime.makeHandle(std::move(self)), runtime);
  }
  return self->isExtensible();
}

bool JSObject::isSealed(PseudoHandle<JSObject> self, Runtime &runtime) {
  if (self->flags_.sealed)
    return true;
  if (!self->flags_.noExtend)
    return false;

  auto selfHandle = runtime.makeHandle(std::move(self));

  if (!HiddenClass::areAllNonConfigurable(
          runtime.makeHandle(selfHandle->clazz_), runtime)) {
    return false;
  }

  if (!checkAllOwnIndexed(
          *selfHandle,
          runtime,
          ObjectVTable::CheckAllOwnIndexedMode::NonConfigurable)) {
    return false;
  }

  // Now that we know we are sealed, set the flag.
  selfHandle->flags_.sealed = true;
  return true;
}

bool JSObject::isFrozen(PseudoHandle<JSObject> self, Runtime &runtime) {
  if (self->flags_.frozen)
    return true;
  if (!self->flags_.noExtend)
    return false;

  auto selfHandle = runtime.makeHandle(std::move(self));

  if (!HiddenClass::areAllReadOnly(
          runtime.makeHandle(selfHandle->clazz_), runtime)) {
    return false;
  }

  if (!checkAllOwnIndexed(
          *selfHandle,
          runtime,
          ObjectVTable::CheckAllOwnIndexedMode::ReadOnly)) {
    return false;
  }

  // Now that we know we are sealed, set the flag.
  selfHandle->flags_.frozen = true;
  selfHandle->flags_.sealed = true;
  return true;
}

CallResult<bool> JSObject::addOwnProperty(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    SymbolID name,
    DefinePropertyFlags dpFlags,
    Handle<> valueOrAccessor,
    PropOpFlags opFlags) {
  /// Can we add more properties?
  if (!selfHandle->isExtensible() && !opFlags.getInternalForce()) {
    if (opFlags.getThrowOnError()) {
      return runtime.raiseTypeError(
          TwineChar16("Cannot add new property '") +
          runtime.getIdentifierTable().getStringViewForDev(runtime, name) +
          "'");
    }
    return false;
  }

  PropertyFlags flags{};

  // Accessors don't set writeable.
  if (dpFlags.isAccessor()) {
    dpFlags.setWritable = 0;
    flags.accessor = 1;
  }

  // Override the default flags if specified.
  if (dpFlags.setEnumerable)
    flags.enumerable = dpFlags.enumerable;
  if (dpFlags.setWritable)
    flags.writable = dpFlags.writable;
  if (dpFlags.setConfigurable)
    flags.configurable = dpFlags.configurable;
  flags.internalSetter = dpFlags.enableInternalSetter;

  if (LLVM_UNLIKELY(
          addOwnPropertyImpl(
              selfHandle, runtime, name, flags, valueOrAccessor) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return true;
}

ExecutionStatus JSObject::addOwnPropertyImpl(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    SymbolID name,
    PropertyFlags propertyFlags,
    Handle<> valueOrAccessor) {
  assert(
      !selfHandle->flags_.proxyObject &&
      "Internal properties cannot be added to Proxy objects");
  // Add a new property to the class.
  // TODO: if we check for OOM here in the future, we must undo the slot
  // allocation.
  auto addResult = HiddenClass::addProperty(
      runtime.makeHandle(selfHandle->clazz_), runtime, name, propertyFlags);
  if (LLVM_UNLIKELY(addResult == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  selfHandle->clazz_.setNonNull(runtime, *addResult->first, runtime.getHeap());

  allocateNewSlotStorage(
      selfHandle, runtime, addResult->second, valueOrAccessor);

  // If this is an index-like property, we need to clear the fast path flags.
  if (LLVM_UNLIKELY(
          selfHandle->clazz_.getNonNull(runtime)->getHasIndexLikeProperties()))
    selfHandle->flags_.fastIndexProperties = false;

  return ExecutionStatus::RETURNED;
}

CallResult<bool> JSObject::updateOwnProperty(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    SymbolID name,
    HiddenClass::PropertyPos propertyPos,
    NamedPropertyDescriptor desc,
    const DefinePropertyFlags dpFlags,
    Handle<> valueOrAccessor,
    PropOpFlags opFlags) {
  auto updateStatus = checkPropertyUpdate(
      runtime,
      desc.flags,
      dpFlags,
      getNamedSlotValueUnsafe(selfHandle.get(), runtime, desc)
          .unboxToHV(runtime),
      valueOrAccessor,
      opFlags);
  if (updateStatus == ExecutionStatus::EXCEPTION)
    return ExecutionStatus::EXCEPTION;
  if (updateStatus->first == PropertyUpdateStatus::failed)
    return false;

  // If the property flags changed, update them.
  if (updateStatus->second != desc.flags) {
    desc.flags = updateStatus->second;
    auto newClazz = HiddenClass::updateProperty(
        runtime.makeHandle(selfHandle->clazz_),
        runtime,
        propertyPos,
        desc.flags);
    selfHandle->clazz_.setNonNull(runtime, *newClazz, runtime.getHeap());
  }

  if (updateStatus->first == PropertyUpdateStatus::done)
    return true;
  assert(
      updateStatus->first == PropertyUpdateStatus::needSet &&
      "unexpected PropertyUpdateStatus");

  if (dpFlags.setValue) {
    if (LLVM_LIKELY(!desc.flags.internalSetter)) {
      auto shv = SmallHermesValue::encodeHermesValue(*valueOrAccessor, runtime);
      setNamedSlotValueUnsafe(selfHandle.get(), runtime, desc, shv);
    } else {
      return internalSetter(
          selfHandle, runtime, name, desc, valueOrAccessor, opFlags);
    }
  } else if (dpFlags.isAccessor()) {
    auto shv = SmallHermesValue::encodeHermesValue(*valueOrAccessor, runtime);
    setNamedSlotValueUnsafe(selfHandle.get(), runtime, desc, shv);
  } else {
    // If checkPropertyUpdate() returned needSet, but there is no value or
    // accessor, clear the value.
    setNamedSlotValueUnsafe(
        selfHandle.get(),
        runtime,
        desc,
        SmallHermesValue::encodeUndefinedValue());
  }

  return true;
}

CallResult<std::pair<JSObject::PropertyUpdateStatus, PropertyFlags>>
JSObject::checkPropertyUpdate(
    Runtime &runtime,
    const PropertyFlags currentFlags,
    DefinePropertyFlags dpFlags,
    const HermesValue curValueOrAccessor,
    Handle<> valueOrAccessor,
    PropOpFlags opFlags) {
  // 8.12.9 [5] Return true, if every field in Desc is absent.
  if (dpFlags.isEmpty())
    return std::make_pair(PropertyUpdateStatus::done, currentFlags);

  assert(
      (!dpFlags.isAccessor() || (!dpFlags.setWritable && !dpFlags.writable)) &&
      "can't set both accessor and writable");
  assert(
      !dpFlags.enableInternalSetter &&
      "cannot change the value of internalSetter");

  // 8.12.9 [6] Return true, if every field in Desc also occurs in current and
  // the value of every field in Desc is the same value as the corresponding
  // field in current when compared using the SameValue algorithm (9.12).
  // TODO: this would probably be much more efficient with bitmasks.
  if ((!dpFlags.setEnumerable ||
       dpFlags.enumerable == currentFlags.enumerable) &&
      (!dpFlags.setConfigurable ||
       dpFlags.configurable == currentFlags.configurable)) {
    if (dpFlags.isAccessor()) {
      if (currentFlags.accessor) {
        auto *curAccessor = vmcast<PropertyAccessor>(curValueOrAccessor);
        auto *newAccessor = vmcast<PropertyAccessor>(valueOrAccessor.get());

        if ((!dpFlags.setGetter ||
             curAccessor->getter == newAccessor->getter) &&
            (!dpFlags.setSetter ||
             curAccessor->setter == newAccessor->setter)) {
          return std::make_pair(PropertyUpdateStatus::done, currentFlags);
        }
      }
    } else {
      if (!currentFlags.accessor &&
          (!dpFlags.setValue ||
           isSameValue(curValueOrAccessor, valueOrAccessor.get())) &&
          (!dpFlags.setWritable || dpFlags.writable == currentFlags.writable)) {
        return std::make_pair(PropertyUpdateStatus::done, currentFlags);
      }
    }
  }

  // 8.12.9 [7]
  // If the property is not configurable, some aspects are not changeable.
  if (!currentFlags.configurable) {
    // Trying to change non-configurable to configurable?
    if (dpFlags.configurable) {
      if (opFlags.getThrowOnError()) {
        return runtime.raiseTypeError(
            "property is not configurable"); // TODO: better message.
      }
      return std::make_pair(PropertyUpdateStatus::failed, PropertyFlags{});
    }

    // Trying to change the enumerability of non-configurable property?
    if (dpFlags.setEnumerable &&
        dpFlags.enumerable != currentFlags.enumerable) {
      if (opFlags.getThrowOnError()) {
        return runtime.raiseTypeError(
            "property is not configurable"); // TODO: better message.
      }
      return std::make_pair(PropertyUpdateStatus::failed, PropertyFlags{});
    }
  }

  PropertyFlags newFlags = currentFlags;

  // 8.12.9 [8] If IsGenericDescriptor(Desc) is true, then no further validation
  // is required.
  if (!(dpFlags.setValue || dpFlags.setWritable || dpFlags.setGetter ||
        dpFlags.setSetter)) {
    // Do nothing
  }
  // 8.12.9 [9]
  // Changing between accessor and data descriptor?
  else if (currentFlags.accessor != dpFlags.isAccessor()) {
    if (!currentFlags.configurable) {
      if (opFlags.getThrowOnError()) {
        return runtime.raiseTypeError(
            "property is not configurable"); // TODO: better message.
      }
      return std::make_pair(PropertyUpdateStatus::failed, PropertyFlags{});
    }

    // If we change from accessor to data descriptor, Preserve the existing
    // values of the converted property’s [[Configurable]] and [[Enumerable]]
    // attributes and set the rest of the property’s attributes to their default
    // values.
    // If it's the other way around, since the accessor doesn't have the
    // [[Writable]] attribute, do nothing.
    newFlags.writable = 0;

    // If we are changing from accessor to non-accessor, we must set a new
    // value.
    if (!dpFlags.isAccessor())
      dpFlags.setValue = 1;
  }
  // 8.12.9 [10] if both are data descriptors.
  else if (!currentFlags.accessor) {
    if (!currentFlags.configurable) {
      if (!currentFlags.writable) {
        // If the current property is not writable, but the new one is.
        if (dpFlags.writable) {
          if (opFlags.getThrowOnError()) {
            return runtime.raiseTypeError(
                "property is not configurable"); // TODO: better message.
          }
          return std::make_pair(PropertyUpdateStatus::failed, PropertyFlags{});
        }

        // If we are setting a different value.
        if (dpFlags.setValue &&
            !isSameValue(curValueOrAccessor, valueOrAccessor.get())) {
          if (opFlags.getThrowOnError()) {
            return runtime.raiseTypeError(
                "property is not writable"); // TODO: better message.
          }
          return std::make_pair(PropertyUpdateStatus::failed, PropertyFlags{});
        }
      }
    }
  }
  // 8.12.9 [11] Both are accessors.
  else {
    auto *curAccessor = vmcast<PropertyAccessor>(curValueOrAccessor);
    auto *newAccessor = vmcast<PropertyAccessor>(valueOrAccessor.get());

    // If not configurable, make sure that nothing is changing.
    if (!currentFlags.configurable) {
      if ((dpFlags.setGetter && newAccessor->getter != curAccessor->getter) ||
          (dpFlags.setSetter && newAccessor->setter != curAccessor->setter)) {
        if (opFlags.getThrowOnError()) {
          return runtime.raiseTypeError(
              "property is not configurable"); // TODO: better message.
        }
        return std::make_pair(PropertyUpdateStatus::failed, PropertyFlags{});
      }
    }

    // If not setting the getter or the setter, re-use the current one.
    if (!dpFlags.setGetter)
      newAccessor->getter.set(runtime, curAccessor->getter, runtime.getHeap());
    if (!dpFlags.setSetter)
      newAccessor->setter.set(runtime, curAccessor->setter, runtime.getHeap());
  }

  // 8.12.9 [12] For each attribute field of Desc that is present, set the
  // correspondingly named attribute of the property named P of object O to the
  // value of the field.
  if (dpFlags.setEnumerable)
    newFlags.enumerable = dpFlags.enumerable;
  if (dpFlags.setWritable)
    newFlags.writable = dpFlags.writable;
  if (dpFlags.setConfigurable)
    newFlags.configurable = dpFlags.configurable;

  if (dpFlags.setValue)
    newFlags.accessor = false;
  else if (dpFlags.isAccessor())
    newFlags.accessor = true;
  else
    return std::make_pair(PropertyUpdateStatus::done, newFlags);

  return std::make_pair(PropertyUpdateStatus::needSet, newFlags);
}

CallResult<bool> JSObject::internalSetter(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    SymbolID name,
    NamedPropertyDescriptor /*desc*/,
    Handle<> value,
    PropOpFlags opFlags) {
  if (vmisa<JSArray>(selfHandle.get())) {
    if (name == Predefined::getSymbolID(Predefined::length)) {
      return JSArray::setLength(
          Handle<JSArray>::vmcast(selfHandle), runtime, value, opFlags);
    }
  }

  llvm_unreachable("unhandled property in Object::internalSetter()");
}

namespace {

/// Helper function to add all the property names of an object to an
/// array, starting at the given index. Only enumerable properties are
/// included. Returns the index after the last property added, but...
CallResult<uint32_t> appendAllPropertyNames(
    Handle<JSObject> obj,
    Runtime &runtime,
    MutableHandle<BigStorage> &arr,
    uint32_t beginIndex) {
  uint32_t size = beginIndex;
  // We know that duplicate property names can only exist between objects in
  // the prototype chain. Hence there should not be duplicated properties
  // before we start to look at any prototype.
  bool needDedup = false;
  MutableHandle<> prop(runtime);
  MutableHandle<JSObject> head(runtime, obj.get());
  MutableHandle<StringPrimitive> tmpVal{runtime};
  while (head.get()) {
    GCScope gcScope(runtime);

    // enumerableProps will contain all enumerable own properties from obj.
    // Impl note: this is the only place where getOwnPropertyKeys will be
    // called without IncludeNonEnumerable on a Proxy.  Everywhere else,
    // trap ordering is specified but ES9 13.7.5.15 says "The mechanics and
    // order of enumerating the properties is not specified", which is
    // unusual.
    auto cr =
        JSObject::getOwnPropertyNames(head, runtime, true /* onlyEnumerable */);
    if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    auto enumerableProps = *cr;
    auto marker = gcScope.createMarker();
    for (unsigned i = 0, e = enumerableProps->getEndIndex(); i < e; ++i) {
      gcScope.flushToMarker(marker);
      prop = enumerableProps->at(runtime, i).unboxToHV(runtime);
      if (!needDedup) {
        // If no dedup is needed, add it directly.
        if (LLVM_UNLIKELY(
                BigStorage::push_back(arr, runtime, prop) ==
                ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
        ++size;
        continue;
      }
      // Otherwise loop through all existing properties and check if we
      // have seen it before.
      bool dupFound = false;
      if (prop->isNumber()) {
        for (uint32_t j = beginIndex; j < size && !dupFound; ++j) {
          HermesValue val = arr->at(runtime, j);
          if (val.isNumber()) {
            dupFound = val.getNumber() == prop->getNumber();
          } else {
            // val is string, prop is number.
            tmpVal = val.getString();
            auto valNum = toArrayIndex(
                StringPrimitive::createStringView(runtime, tmpVal));
            dupFound = valNum && valNum.getValue() == prop->getNumber();
          }
        }
      } else {
        for (uint32_t j = beginIndex; j < size && !dupFound; ++j) {
          HermesValue val = arr->at(runtime, j);
          if (val.isNumber()) {
            // val is number, prop is string.
            auto propNum = toArrayIndex(StringPrimitive::createStringView(
                runtime, Handle<StringPrimitive>::vmcast(prop)));
            dupFound = propNum && (propNum.getValue() == val.getNumber());
          } else {
            dupFound = val.getString()->equals(prop->getString());
          }
        }
      }
      if (LLVM_LIKELY(!dupFound)) {
        if (LLVM_UNLIKELY(
                BigStorage::push_back(arr, runtime, prop) ==
                ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
        ++size;
      }
    }
    // Continue to follow the prototype chain.
    CallResult<PseudoHandle<JSObject>> parentRes =
        JSObject::getPrototypeOf(head, runtime);
    if (LLVM_UNLIKELY(parentRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    head = parentRes->get();
    needDedup = true;
  }
  return size;
}

/// Adds the hidden classes of the prototype chain of obj to arr,
/// starting with the prototype of obj at index 0, etc., and
/// terminates with null.
///
/// \param obj The object whose prototype chain should be output
/// \param[out] arr The array where the classes will be appended. This
/// array is cleared if any object is unsuitable for caching.
ExecutionStatus setProtoClasses(
    Runtime &runtime,
    Handle<JSObject> obj,
    MutableHandle<BigStorage> &arr) {
  // Layout of a JSArray stored in the for-in cache:
  // [class(proto(obj)), class(proto(proto(obj))), ..., null, prop0, prop1, ...]

  if (!obj->shouldCacheForIn(runtime)) {
    arr->clear(runtime);
    return ExecutionStatus::RETURNED;
  }
  MutableHandle<JSObject> head(runtime, obj->getParent(runtime));
  MutableHandle<> clazz(runtime);
  GCScopeMarkerRAII marker{runtime};
  while (head.get()) {
    if (!head->shouldCacheForIn(runtime)) {
      arr->clear(runtime);
      return ExecutionStatus::RETURNED;
    }
    if (JSObject::Helper::flags(*head).lazyObject) {
      // Ensure all properties have been initialized before caching the hidden
      // class. Not doing this will result in changes to the hidden class
      // when getOwnPropertyKeys is called later.
      JSObject::initializeLazyObject(runtime, head);
    }
    clazz = HermesValue::encodeObjectValue(head->getClass(runtime));
    if (LLVM_UNLIKELY(
            BigStorage::push_back(arr, runtime, clazz) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    head = head->getParent(runtime);
    marker.flush();
  }
  clazz = HermesValue::encodeNullValue();
  return BigStorage::push_back(arr, runtime, clazz);
}

/// Verifies that the classes of obj's prototype chain still matches those
/// previously prefixed to arr by setProtoClasses.
///
/// \param obj The object whose prototype chain should be verified
/// \param arr Array previously populated by setProtoClasses
/// \return The index after the terminating null if everything matches,
/// otherwise 0.
uint32_t matchesProtoClasses(
    Runtime &runtime,
    Handle<JSObject> obj,
    Handle<BigStorage> arr) {
  MutableHandle<JSObject> head(runtime, obj->getParent(runtime));
  uint32_t i = 0;
  while (head.get()) {
    HermesValue protoCls = arr->at(runtime, i++);
    if (protoCls.isNull() || protoCls.getObject() != head->getClass(runtime) ||
        head->isProxyObject()) {
      return 0;
    }
    head = head->getParent(runtime);
  }
  // The chains must both end at the same point.
  if (head || !arr->at(runtime, i++).isNull()) {
    return 0;
  }
  assert(i > 0 && "success should be positive");
  return i;
}

} // namespace

CallResult<Handle<BigStorage>> getForInPropertyNames(
    Runtime &runtime,
    Handle<JSObject> obj,
    uint32_t &beginIndex,
    uint32_t &endIndex) {
  Handle<HiddenClass> clazz(runtime, obj->getClass(runtime));

  // Fast case: Check the cache.
  MutableHandle<BigStorage> arr(runtime);
  if (obj->shouldCacheForIn(runtime)) {
    arr = clazz->getForInCache(runtime);
    if (arr) {
      beginIndex = matchesProtoClasses(runtime, obj, arr);
      if (beginIndex) {
        // Cache is valid for this object, so use it.
        endIndex = arr->size(runtime);
        return arr;
      }
      // Invalid for this object. We choose to clear the cache since the
      // changes to the prototype chain probably affect other objects too.
      clazz->clearForInCache(runtime);
      // Clear arr to slightly reduce risk of OOM from allocation below.
      arr = nullptr;
    }
  }

  // Slow case: Build the array of properties.
  auto ownPropEstimate = clazz->getNumProperties();
  auto arrRes = obj->shouldCacheForIn(runtime)
      ? BigStorage::createLongLived(runtime, ownPropEstimate)
      : BigStorage::create(runtime, ownPropEstimate);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  arr = std::move(*arrRes);
  if (setProtoClasses(runtime, obj, arr) == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  beginIndex = arr->size(runtime);
  // If obj or any of its prototypes are unsuitable for caching, then
  // beginIndex is 0 and we return an array with only the property names.
  bool canCache = beginIndex;
  auto end = appendAllPropertyNames(obj, runtime, arr, beginIndex);
  if (end == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  endIndex = *end;
  // Avoid degenerate memory explosion: if > 75% of the array is properties
  // or classes from prototypes, then don't cache it.
  const bool tooMuchProto = *end / 4 > ownPropEstimate;
  if (canCache && !tooMuchProto) {
    assert(beginIndex > 0 && "cached array must start with proto classes");
#ifdef HERMES_SLOW_DEBUG
    assert(beginIndex == matchesProtoClasses(runtime, obj, arr) && "matches");
#endif
    clazz->setForInCache(*arr, runtime);
  }
  return arr;
}

} // namespace vm
} // namespace hermes
