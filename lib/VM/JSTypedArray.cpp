/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSTypedArray.h"

#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/Callable.h"

#pragma GCC diagnostic push

#ifdef HERMES_COMPILER_SUPPORTS_WSHORTEN_64_TO_32
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif
namespace hermes {
namespace vm {

/// @name JSTypedArrayBase
/// @{

JSTypedArrayBase::JSTypedArrayBase(
    Runtime &runtime,
    Handle<JSObject> parent,
    Handle<HiddenClass> clazz)
    : JSObject(runtime, *parent, *clazz),
      buffer_(nullptr),
      length_(0),
      offset_(0) {
  flags_.indexedStorage = true;
  flags_.fastIndexProperties = true;
}

void TypedArrayBaseBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSTypedArrayBase>());
  JSObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const JSTypedArrayBase *>(cell);
  mb.addField("buffer", &self->buffer_);
}

std::pair<uint32_t, uint32_t> JSTypedArrayBase::_getOwnIndexedRangeImpl(
    JSObject *selfObj,
    Runtime &) {
  auto *self = vmcast<JSTypedArrayBase>(selfObj);
  return {0, self->getLength()};
}

bool JSTypedArrayBase::_haveOwnIndexedImpl(
    JSObject *selfObj,
    Runtime &,
    uint32_t index) {
  auto *self = vmcast<JSTypedArrayBase>(selfObj);
  // Check whether the index is within the storage.
  return index < self->getLength();
}

OptValue<PropertyFlags> JSTypedArrayBase::_getOwnIndexedPropertyFlagsImpl(
    JSObject *selfObj,
    Runtime &runtime,
    uint32_t index) {
  auto *self = vmcast<JSTypedArrayBase>(selfObj);
  // Check whether the index is within the storage.
  if (LLVM_UNLIKELY(index >= self->getLength())) {
    return llvh::None;
  }
  PropertyFlags indexedElementFlags;
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

bool JSTypedArrayBase::_deleteOwnIndexedImpl(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    uint32_t index) {
  // Opposite of _haveOwnIndexedImpl.  This is not specified as such,
  // but is a consequence of 9.1.10.1 OrdinaryDelete on TypedArrays.
  // Informally, because elements of typed arrays are
  // non-configurable, delete never changes anything, and returns true
  // if the element does not exist, and false if it does.
  return !_haveOwnIndexedImpl(*selfHandle, runtime, index);
}

bool JSTypedArrayBase::_checkAllOwnIndexedImpl(
    JSObject *selfObj,
    Runtime &,
    ObjectVTable::CheckAllOwnIndexedMode /*mode*/) {
  auto *self = vmcast<JSTypedArrayBase>(selfObj);

  // If we have any indexed properties at all, they don't satisfy the
  // requirements.
  return self->getLength() == 0;
}

ExecutionStatus JSTypedArrayBase::validateTypedArray(
    Runtime &runtime,
    Handle<> thisArg,
    bool checkAttached) {
  auto self = Handle<JSTypedArrayBase>::dyn_vmcast(thisArg);
  if (!self) {
    return runtime.raiseTypeError(
        "A TypedArray function was called on a non TypedArray");
  }
  if (checkAttached && !self->attached(runtime)) {
    return runtime.raiseTypeError(
        "A TypedArray function was called on a detached TypedArray");
  }
  return ExecutionStatus::RETURNED;
}

uint8_t JSTypedArrayBase::getByteWidth() const {
  static constexpr uint8_t widths[] = {
#define TYPED_ARRAY(name, type) sizeof(type),
#include "hermes/VM/TypedArrays.def"
#undef TYPED_ARRAY
  };
  static constexpr size_t firstKind =
      static_cast<size_t>(CellKind::TypedArrayBaseKind_first);
  return widths[static_cast<size_t>(getKind()) - firstKind];
}

CallResult<Handle<JSTypedArrayBase>> JSTypedArrayBase::allocate(
    Runtime &runtime,
    size_type length) {
  using AllocateFn = CallResult<Handle<JSTypedArrayBase>>(Runtime &, size_type);
  static constexpr AllocateFn *allocateFns[] = {
#define TYPED_ARRAY(name, type) name##Array::allocate,
#include "hermes/VM/TypedArrays.def"
#undef TYPED_ARRAY
  };
  static constexpr size_t firstKind =
      static_cast<size_t>(CellKind::TypedArrayBaseKind_first);
  return allocateFns[static_cast<size_t>(getKind()) - firstKind](
      runtime, length);
}

CallResult<Handle<JSTypedArrayBase>> JSTypedArrayBase::allocateToSameBuffer(
    Runtime &runtime,
    Handle<JSTypedArrayBase> src,
    size_type beginIndex,
    size_type endIndex) {
  auto result =
      JSTypedArrayBase::allocateSpecies(runtime, src, endIndex - beginIndex);
  if (result == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto newArr = result.getValue();
  auto beginScaled = beginIndex * src->getByteWidth();
  auto endScaled = endIndex * src->getByteWidth();
  if (!src->attached(runtime)) {
    return runtime.raiseTypeError("Cannot allocate from a detached TypedArray");
  }
  setBuffer(
      runtime,
      *newArr,
      src->getBuffer(runtime),
      beginScaled + src->getByteOffset(),
      endScaled - beginScaled,
      src->getByteWidth());
  return Handle<JSTypedArrayBase>::vmcast(newArr);
}

CallResult<Handle<JSTypedArrayBase>> JSTypedArrayBase::allocateSpecies(
    Runtime &runtime,
    Handle<JSTypedArrayBase> self,
    size_type length) {
  using AllocateSpeciesFn = CallResult<Handle<JSTypedArrayBase>>(
      Handle<JSTypedArrayBase>, Runtime &, size_type);
  static constexpr AllocateSpeciesFn *allocateFns[] = {
#define TYPED_ARRAY(name, type) name##Array::allocateSpecies,
#include "hermes/VM/TypedArrays.def"
#undef TYPED_ARRAY
  };
  static constexpr size_t firstKind =
      static_cast<size_t>(CellKind::TypedArrayBaseKind_first);
  return allocateFns[static_cast<size_t>(self->getKind()) - firstKind](
      self, runtime, length);
}

ExecutionStatus JSTypedArrayBase::createBuffer(
    Runtime &runtime,
    Handle<JSTypedArrayBase> selfObj,
    uint64_t length) {
  assert(selfObj);

  auto tmpbuf = runtime.makeHandle(JSArrayBuffer::create(
      runtime, Handle<JSObject>::vmcast(&runtime.arrayBufferPrototype)));

  // Ensure that the buffer size in bytes will not overflow
  // JSArrayBuffer::size_type (maybe not same as JSTypedArrayBase::size_type).
  if (length > (std::numeric_limits<JSArrayBuffer::size_type>::max() /
                selfObj->getByteWidth())) {
    return runtime.raiseRangeError(
        "Cannot allocate a data block for the ArrayBuffer");
  }
  JSArrayBuffer::size_type bufferSize = length * selfObj->getByteWidth();
  if (JSArrayBuffer::createDataBlock(runtime, tmpbuf, bufferSize) ==
      ExecutionStatus::EXCEPTION) {
    // Failed to allocate, don't modify what it currently points to.
    return ExecutionStatus::EXCEPTION;
  }
  // Commit the change.
  setBuffer(runtime, *selfObj, *tmpbuf, 0, bufferSize, selfObj->getByteWidth());
  return ExecutionStatus::RETURNED;
}

ExecutionStatus JSTypedArrayBase::setToCopyOfBuffer(
    Runtime &runtime,
    Handle<JSTypedArrayBase> dst,
    JSArrayBuffer::size_type dstByteOffset,
    Handle<JSArrayBuffer> src,
    JSArrayBuffer::size_type srcByteOffset,
    JSArrayBuffer::size_type count) {
  assert(src && "Must be cloned from an existing buffer");
  auto possibleArr = JSArrayBuffer::clone(runtime, src, srcByteOffset, count);
  if (possibleArr == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto arr = possibleArr.getValue();
  assert(arr->size() == count && "Created buffer is not of the correct size");
  setBuffer(
      runtime, *dst, *arr, dstByteOffset, arr->size(), dst->getByteWidth());
  return ExecutionStatus::RETURNED;
}

ExecutionStatus JSTypedArrayBase::setToCopyOfTypedArray(
    Runtime &runtime,
    Handle<JSTypedArrayBase> dst,
    size_type dstIndex,
    Handle<JSTypedArrayBase> src,
    size_type srcIndex,
    size_type count) {
  if (count == 0) {
    // Don't do anything if the copy size is 0.
    return ExecutionStatus::RETURNED;
  }
  assert(
      dst->getBuffer(runtime)->getDataBlock(runtime) !=
          src->getBuffer(runtime)->getDataBlock(runtime) &&
      "Must call setToCopyOfTypedArray with two TypedArrays with different "
      "backing ArrayBuffers");
  if (dst->getKind() == src->getKind()) {
    // Preserve byte-wise equality if they're the same type.
    JSTypedArrayBase::setToCopyOfBytes(
        runtime, dst, dstIndex, src, srcIndex, count);
  } else {
    // Else must do type conversions.
    MutableHandle<> storage(runtime);
    for (auto k = srcIndex; k < srcIndex + count; ++k) {
      storage =
          JSObject::getOwnIndexed(createPseudoHandle(src.get()), runtime, k);
      if (JSObject::setOwnIndexed(dst, runtime, dstIndex++, storage) ==
          ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
    }
  }
  return ExecutionStatus::RETURNED;
}

void JSTypedArrayBase::setToCopyOfBytes(
    Runtime &runtime,
    Handle<JSTypedArrayBase> dst,
    size_type dstIndex,
    Handle<JSTypedArrayBase> src,
    size_type srcIndex,
    size_type count) {
  assert(
      dst->getKind() == src->getKind() &&
      "Cannot copy bytes across separate typed arrays");
  // They must be of the same type, which means their byte widths must be the
  // same as well.
  assert(
      srcIndex + count <= src->getLength() &&
      "Cannot read that many elements from src");
  assert(
      dstIndex + count <= dst->getLength() &&
      "Cannot write that many elements to dest ");
  JSArrayBuffer::copyDataBlockBytes(
      runtime,
      dst->getBuffer(runtime),
      dstIndex * dst->getByteWidth() + dst->getByteOffset(),
      src->getBuffer(runtime),
      srcIndex * src->getByteWidth() + src->getByteOffset(),
      count * dst->getByteWidth());
}

void JSTypedArrayBase::setBuffer(
    Runtime &runtime,
    JSTypedArrayBase *self,
    JSArrayBuffer *buf,
    size_type offset,
    size_type size,
    uint8_t byteWidth) {
  assert(self && buf && "Cannot pass in a nullptr to setBuffer");
  assert(offset + size <= buf->size());
  assert(
      offset % byteWidth == 0 && "offset must be aligned by the element size");
  assert(size % byteWidth == 0 && "size must be aligned by the element size");
  assert(
      self->getByteWidth() == byteWidth &&
      "Cannot set to a buffer of a different byte width");
  self->buffer_.setNonNull(runtime, buf, runtime.getHeap());
  self->offset_ = offset;
  self->length_ = size / byteWidth;
}

/// @}

template <typename T, CellKind C>
const ObjectVTable JSTypedArray<T, C>::vt{
    VTable(C, cellSize<JSTypedArray<T, C>>()),
    _getOwnIndexedRangeImpl,
    _haveOwnIndexedImpl,
    _getOwnIndexedPropertyFlagsImpl,
    _getOwnIndexedImpl,
    _setOwnIndexedImpl,
    _deleteOwnIndexedImpl,
    _checkAllOwnIndexedImpl,
};

#define TYPED_ARRAY(name, type)                                          \
  void name##ArrayBuildMeta(const GCCell *cell, Metadata::Builder &mb) { \
    TypedArrayBaseBuildMeta(cell, mb);                                   \
    mb.setVTable(&name##Array::vt);                                      \
  }
#include "hermes/VM/TypedArrays.def"
#undef TYPED_ARRAY

template <typename T, CellKind C>
CallResult<Handle<JSTypedArrayBase>> JSTypedArray<T, C>::allocate(
    Runtime &runtime,
    size_type length) {
  Handle<JSTypedArrayBase> ta = runtime.makeHandle(JSTypedArray<T, C>::create(
      runtime, JSTypedArray<T, C>::getPrototype(runtime)));
  if (JSTypedArrayBase::createBuffer(runtime, ta, length) ==
      ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  return ta;
}

template <typename T, CellKind C>
CallResult<Handle<JSTypedArrayBase>> JSTypedArray<T, C>::allocateSpecies(
    Handle<JSTypedArrayBase> self,
    Runtime &runtime,
    size_type length) {
  auto defaultConstructor = JSTypedArray<T, C>::getConstructor(runtime);
  auto possibleCons = speciesConstructor(self, runtime, defaultConstructor);
  if (possibleCons == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto callRes = Callable::executeConstruct1(
      *possibleCons,
      runtime,
      runtime.makeHandle(HermesValue::encodeNumberValue(length)));
  if (callRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto obj = runtime.makeHandle<JSObject>(callRes->get());
  // validate that the constructed object is a TypedArray.
  if (JSTypedArrayBase::validateTypedArray(runtime, obj) ==
      ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  return Handle<JSTypedArrayBase>::vmcast(obj);
}

template <typename T, CellKind C>
PseudoHandle<JSTypedArray<T, C>> JSTypedArray<T, C>::create(
    Runtime &runtime,
    Handle<JSObject> parentHandle) {
  auto *cell = runtime.makeAFixed<JSTypedArray<T, C>>(
      runtime,
      parentHandle,
      runtime.getHiddenClassForPrototype(
          *parentHandle, numOverlapSlots<JSTypedArray>()));
  return JSObjectInit::initToPseudoHandle(runtime, cell);
  // NOTE: If any fields are ever added beyond the base class, then the
  // *BuildMeta functions must be updated to call addJSObjectOverlapSlots.
  static_assert(
      sizeof(JSTypedArray<T, C>) == sizeof(JSTypedArrayBase),
      "must update BuildMeta");
}

/// @name Specializations for specific types
/// @{

#define TYPED_ARRAY(name, type)                                    \
  template <>                                                      \
  SymbolID JSTypedArray<type, CellKind::name##ArrayKind>::getName( \
      Runtime &runtime) {                                          \
    return Predefined::getSymbolID(Predefined::name##Array);       \
  }
#include "hermes/VM/TypedArrays.def"

#define TYPED_ARRAY(name, type)                                     \
  template <>                                                       \
  Handle<JSObject>                                                  \
  JSTypedArray<type, CellKind::name##ArrayKind>::getPrototype(      \
      const Runtime &runtime) {                                     \
    return Handle<JSObject>::vmcast(&runtime.name##ArrayPrototype); \
  }
#include "hermes/VM/TypedArrays.def"

#define TYPED_ARRAY(name, type)                                       \
  template <>                                                         \
  Handle<Callable>                                                    \
  JSTypedArray<type, CellKind::name##ArrayKind>::getConstructor(      \
      const Runtime &runtime) {                                       \
    return Handle<Callable>::vmcast(&runtime.name##ArrayConstructor); \
  }
#include "hermes/VM/TypedArrays.def"

/// @}

template <typename T, CellKind C>
JSTypedArray<T, C>::JSTypedArray(
    Runtime &runtime,
    Handle<JSObject> parent,
    Handle<HiddenClass> clazz)
    : JSTypedArrayBase(runtime, parent, clazz) {}

template <>
int64_t JSTypedArray<int64_t, CellKind::BigInt64ArrayKind>::toDestType(
    const HermesValue &numeric) {
  assert(numeric.isBigInt() && "expected bigint");
  auto digits = numeric.getBigInt()->getDigits();
  return digits.size() == 0 ? 0ll : static_cast<int64_t>(digits[0]);
}

template <>
uint64_t JSTypedArray<uint64_t, CellKind::BigUint64ArrayKind>::toDestType(
    const HermesValue &numeric) {
  assert(numeric.isBigInt() && "expected bigint");
  auto digits = numeric.getBigInt()->getDigits();
  return digits.size() == 0 ? 0ull : static_cast<uint64_t>(digits[0]);
}

template <typename T>
struct _getOwnRetEncoder {
  static HermesValue encodeMayAlloc(Runtime &, T element) {
    return SafeNumericEncoder<T>::encode(element);
  }
};

template <>
struct _getOwnRetEncoder<int64_t> {
  static HermesValue encodeMayAlloc(Runtime &runtime, int64_t element) {
    auto res = BigIntPrimitive::fromSigned(runtime, element);
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      assert(false && "Failed to encode BigInt in _getOwnRetEncoder<int64_t>");
      runtime.clearThrownValue();
      return HermesValue::encodeUndefinedValue();
    }
    return *res;
  }
};

template <>
struct _getOwnRetEncoder<uint64_t> {
  static HermesValue encodeMayAlloc(Runtime &runtime, uint64_t element) {
    auto res = BigIntPrimitive::fromUnsigned(runtime, element);
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      assert(false && "Failed to encode BigInt in _getOwnRetEncoder<uint64_t>");
      runtime.clearThrownValue();
      return HermesValue::encodeUndefinedValue();
    }
    return *res;
  }
};

template <typename T, CellKind C>
HermesValue JSTypedArray<T, C>::_getOwnIndexedImpl(
    PseudoHandle<JSObject> selfObj,
    Runtime &runtime,
    uint32_t index) {
  NoAllocScope noAllocs{runtime};
  auto *self = vmcast<JSTypedArray>(selfObj.get());

  if (LLVM_UNLIKELY(!self->attached(runtime))) {
    noAllocs.release();
    // NOTE: This should be a TypeError to be fully spec-compliant, but
    // getOwnIndexed is not allowed to return an exception.
    return _getOwnRetEncoder<T>::encodeMayAlloc(runtime, 0);
  }
  if (LLVM_LIKELY(index < self->getLength())) {
    auto elem = self->at(runtime, index);
    noAllocs.release();
    return _getOwnRetEncoder<T>::encodeMayAlloc(runtime, elem);
  }
  return HermesValue::encodeUndefinedValue();
}

template <CellKind>
struct _setOwnValueEncoder {
  static CallResult<HermesValue> encode(Runtime &runtime, Handle<> value) {
    if (LLVM_UNLIKELY(!value->isNumber())) {
      auto res = toNumber_RJS(runtime, value);
      if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
        return ExecutionStatus::EXCEPTION;
      return *res;
    }
    return *value;
  }
};

template <>
struct _setOwnValueEncoder<CellKind::BigInt64ArrayKind> {
  static CallResult<HermesValue> encode(Runtime &runtime, Handle<> value) {
    auto res = toBigInt_RJS(runtime, value);
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    HermesValue prim = *res;
    if (LLVM_UNLIKELY(!prim.isBigInt())) {
      return runtime.raiseTypeErrorForValue(
          "can't convert ", value, " to bigint");
    }
    return prim;
  }
};

template <>
struct _setOwnValueEncoder<CellKind::BigUint64ArrayKind> {
  static CallResult<HermesValue> encode(Runtime &runtime, Handle<> value) {
    auto res = toBigInt_RJS(runtime, value);
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    HermesValue prim = *res;
    if (LLVM_UNLIKELY(!prim.isBigInt())) {
      return runtime.raiseTypeErrorForValue(
          "can't convert ", value, " to bigint");
    }
    return prim;
  }
};

template <typename T, CellKind C>
CallResult<bool> JSTypedArray<T, C>::_setOwnIndexedImpl(
    Handle<JSObject> selfHandle,
    Runtime &runtime,
    uint32_t index,
    Handle<> value) {
  auto typedArrayHandle = Handle<JSTypedArray>::vmcast(selfHandle);
  CallResult<HermesValue> res = _setOwnValueEncoder<C>::encode(runtime, value);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  T destValue = JSTypedArray<T, C>::toDestType(*res);
  if (LLVM_UNLIKELY(!typedArrayHandle->attached(runtime))) {
    return runtime.raiseTypeError(
        "Cannot set a value into a detached ArrayBuffer");
  }
  if (LLVM_LIKELY(index < typedArrayHandle->getLength())) {
    typedArrayHandle->at(runtime, index) = destValue;
  }
  return true;
}

// Since there are a limited number of TypedArray's, forward define them here
// to avoid header file recompile churn.
#define TYPED_ARRAY(name, type) \
  template class JSTypedArray<type, CellKind::name##ArrayKind>;
#include "hermes/VM/TypedArrays.def"

} // namespace vm
} // namespace hermes
