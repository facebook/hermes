/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/JSTypedArray.h"

#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/Callable.h"

#include "llvm/Support/Debug.h"
#define DEBUG_TYPE "serialize"

namespace hermes {
namespace vm {

/// @name JSTypedArrayBase
/// @{

JSTypedArrayBase::JSTypedArrayBase(
    Runtime *runtime,
    const VTable *vt,
    JSObject *parent,
    HiddenClass *clazz)
    : JSObject(runtime, vt, parent, clazz),
      buffer_(nullptr),
      length_(0),
      byteWidth_(0),
      src_(nullptr) {
  flags_.indexedStorage = true;
  flags_.fastIndexProperties = true;
}

void TypedArrayBaseBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  ObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const JSTypedArrayBase *>(cell);
  mb.addField("@buffer", &self->buffer_);
}

std::pair<uint32_t, uint32_t> JSTypedArrayBase::_getOwnIndexedRangeImpl(
    JSObject *selfObj,
    Runtime *) {
  auto *self = vmcast<JSTypedArrayBase>(selfObj);
  return {0, self->getLength()};
}

void TypedArrayBaseSerialize(Serializer &s, const GCCell *cell) {
  LLVM_DEBUG(
      llvm::dbgs() << "Serialize function not implemented for TypedArray\n");
}

void TypedArrayBaseDeserialize(Deserializer &d, CellKind kind) {
  LLVM_DEBUG(
      llvm::dbgs() << "Deserialize function not implemented for TypedArray\n");
}

bool JSTypedArrayBase::_haveOwnIndexedImpl(JSObject *, Runtime *, uint32_t) {
  return true;
}

OptValue<PropertyFlags> JSTypedArrayBase::_getOwnIndexedPropertyFlagsImpl(
    JSObject *selfObj,
    Runtime *runtime,
    uint32_t index) {
  auto *self = vmcast<JSTypedArrayBase>(selfObj);
  // Check whether the index is within the storage.
  if (LLVM_UNLIKELY(index >= self->getLength())) {
    return llvm::None;
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

bool JSTypedArrayBase::_checkAllOwnIndexedImpl(
    JSObject *selfObj,
    Runtime *,
    ObjectVTable::CheckAllOwnIndexedMode /*mode*/) {
  auto *self = vmcast<JSTypedArrayBase>(selfObj);

  // If we have any indexed properties at all, they don't satisfy the
  // requirements.
  return self->getLength() == 0;
}

ExecutionStatus JSTypedArrayBase::validateTypedArray(
    Runtime *runtime,
    Handle<> thisArg,
    bool checkAttached) {
  auto self = Handle<JSTypedArrayBase>::dyn_vmcast(runtime, thisArg);
  if (!self) {
    return runtime->raiseTypeError(
        "A TypedArray function was called on a non TypedArray");
  }
  if (checkAttached && !self->attached(runtime)) {
    return runtime->raiseTypeError(
        "A TypedArray function was called on a detached TypedArray");
  }
  return ExecutionStatus::RETURNED;
}

CallResult<Handle<JSTypedArrayBase>> JSTypedArrayBase::allocateToSameBuffer(
    Runtime *runtime,
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
    return runtime->raiseTypeError(
        "Cannot allocate from a detached TypedArray");
  }
  setBuffer(
      runtime,
      *newArr,
      src->getBuffer(runtime),
      beginScaled + src->getByteOffset(runtime),
      endScaled - beginScaled,
      src->getByteWidth());
  return Handle<JSTypedArrayBase>::vmcast(newArr);
}

ExecutionStatus JSTypedArrayBase::createBuffer(
    Runtime *runtime,
    Handle<JSTypedArrayBase> selfObj,
    size_type length) {
  assert(runtime && selfObj);

  auto arrRes = JSArrayBuffer::create(
      runtime, Handle<JSObject>::vmcast(&runtime->arrayBufferPrototype));
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto tmpbuf = runtime->makeHandle<JSArrayBuffer>(*arrRes);

  auto bufferSize = length * selfObj->getByteWidth();
  if (tmpbuf->createDataBlock(runtime, bufferSize) ==
      ExecutionStatus::EXCEPTION) {
    // Failed to allocate, don't modify what it currently points to.
    return ExecutionStatus::EXCEPTION;
  }
  // Commit the change.
  setBuffer(runtime, *selfObj, *tmpbuf, 0, bufferSize, selfObj->getByteWidth());
  return ExecutionStatus::RETURNED;
}

ExecutionStatus JSTypedArrayBase::setToCopyOfBuffer(
    Runtime *runtime,
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
    Runtime *runtime,
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
      dst->getBuffer(runtime)->getDataBlock() !=
          src->getBuffer(runtime)->getDataBlock() &&
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
      storage = JSObject::getOwnIndexed(*src, runtime, k);
      if (JSObject::setOwnIndexed(dst, runtime, dstIndex++, storage) ==
          ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
    }
  }
  return ExecutionStatus::RETURNED;
}

void JSTypedArrayBase::setToCopyOfBytes(
    Runtime *runtime,
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
      dst->getBuffer(runtime),
      dstIndex * dst->getByteWidth() + dst->getByteOffset(runtime),
      src->getBuffer(runtime),
      srcIndex * src->getByteWidth() + src->getByteOffset(runtime),
      count * dst->getByteWidth());
}

void JSTypedArrayBase::setBuffer(
    Runtime *runtime,
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
  self->buffer_.set(runtime, buf, &runtime->getHeap());
  self->src_ = buf->attached() && buf->size() != 0
      ? buf->getDataBlock() + offset
      : nullptr;
  self->length_ = size / byteWidth;
}

/// @}

template <typename T, CellKind C>
JSTypedArrayBase::JSTypedArrayVTable JSTypedArray<T, C>::vt{
    {
        VTable(C, sizeof(JSTypedArray<T, C>)),
        _getOwnIndexedRangeImpl,
        _haveOwnIndexedImpl,
        _getOwnIndexedPropertyFlagsImpl,
        _getOwnIndexedImpl,
        _setOwnIndexedImpl,
        _deleteOwnIndexedImpl,
        _checkAllOwnIndexedImpl,
    },
    allocate,
    _allocateSpeciesImpl};

#define TYPED_ARRAY(name, type)                                          \
  void name##ArrayBuildMeta(const GCCell *cell, Metadata::Builder &mb) { \
    TypedArrayBaseBuildMeta(cell, mb);                                   \
  }                                                                      \
  void name##ArraySerialize(Serializer &s, const GCCell *cell) {         \
    TypedArrayBaseSerialize(s, cell);                                    \
  }                                                                      \
  void name##ArrayDeserialize(Deserializer &d, CellKind kind) {          \
    TypedArrayBaseDeserialize(d, kind);                                  \
  }
#include "hermes/VM/TypedArrays.def"

template <typename T, CellKind C>
CallResult<Handle<JSTypedArrayBase>> JSTypedArray<T, C>::allocate(
    Runtime *runtime,
    size_type length) {
  auto arrRes = JSTypedArray<T, C>::create(
      runtime, JSTypedArray<T, C>::getPrototype(runtime));
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto ta = Handle<JSTypedArrayBase>::vmcast(runtime->makeHandle(*arrRes));
  if (JSTypedArrayBase::createBuffer(runtime, ta, length) ==
      ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  return ta;
}

template <typename T, CellKind C>
CallResult<Handle<JSTypedArrayBase>> JSTypedArray<T, C>::_allocateSpeciesImpl(
    Handle<JSTypedArrayBase> self,
    Runtime *runtime,
    size_type length) {
  auto defaultConstructor = JSTypedArray<T, C>::getConstructor(runtime);
  auto possibleCons = speciesConstructor(self, runtime, defaultConstructor);
  if (possibleCons == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto callRes = Callable::executeConstruct1(
      *possibleCons,
      runtime,
      runtime->makeHandle(HermesValue::encodeNumberValue(length)));
  if (callRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto obj = runtime->makeHandle<JSObject>(*callRes);
  // validate that the constructed object is a TypedArray.
  if (JSTypedArrayBase::validateTypedArray(runtime, obj) ==
      ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  return Handle<JSTypedArrayBase>::vmcast(obj);
}

template <typename T, CellKind C>
CallResult<HermesValue> JSTypedArray<T, C>::create(
    Runtime *runtime,
    Handle<JSObject> parentHandle) {
  void *mem = runtime->alloc(sizeof(JSTypedArray<T, C>));
  return HermesValue::encodeObjectValue(
      JSObject::allocateSmallPropStorage<NEEDED_PROPERTY_SLOTS>(
          new (mem) JSTypedArray<T, C>(
              runtime,
              *parentHandle,
              runtime->getHiddenClassForPrototypeRaw(*parentHandle))));
}

/// @name Specializations for specific types
/// @{

#define TYPED_ARRAY(name, type)                                    \
  template <>                                                      \
  SymbolID JSTypedArray<type, CellKind::name##ArrayKind>::getName( \
      Runtime *runtime) {                                          \
    return Predefined::getSymbolID(Predefined::name##Array);       \
  }
#include "hermes/VM/TypedArrays.def"

#define TYPED_ARRAY(name, type)                                      \
  template <>                                                        \
  Handle<JSObject>                                                   \
  JSTypedArray<type, CellKind::name##ArrayKind>::getPrototype(       \
      const Runtime *runtime) {                                      \
    return Handle<JSObject>::vmcast(&runtime->name##ArrayPrototype); \
  }
#include "hermes/VM/TypedArrays.def"

#define TYPED_ARRAY(name, type)                                        \
  template <>                                                          \
  Handle<Callable>                                                     \
  JSTypedArray<type, CellKind::name##ArrayKind>::getConstructor(       \
      const Runtime *runtime) {                                        \
    return Handle<Callable>::vmcast(&runtime->name##ArrayConstructor); \
  }
#include "hermes/VM/TypedArrays.def"

/// @}

template <typename T, CellKind C>
JSTypedArray<T, C>::JSTypedArray(
    Runtime *runtime,
    JSObject *parent,
    HiddenClass *clazz)
    : JSTypedArrayBase(runtime, &vt.base.base, parent, clazz) {
  byteWidth_ = sizeof(T);
}

template <typename T, CellKind C>
HermesValue JSTypedArray<T, C>::_getOwnIndexedImpl(
    JSObject *selfObj,
    Runtime *runtime,
    uint32_t index) {
  auto *self = vmcast<JSTypedArray>(selfObj);
  if (LLVM_UNLIKELY(!self->attached(runtime))) {
    // NOTE: This should be a TypeError to be fully spec-compliant, but
    // getOwnIndexed is not allowed to return an exception.
    return HermesValue::encodeNumberValue(0);
  }
  if (LLVM_LIKELY(index < self->getLength())) {
    return SafeNumericEncoder<T>::encode(self->template at<T>(runtime, index));
  }
  return HermesValue::encodeUndefinedValue();
}

template <typename T, CellKind C>
CallResult<bool> JSTypedArray<T, C>::_setOwnIndexedImpl(
    Handle<JSObject> selfHandle,
    Runtime *runtime,
    uint32_t index,
    Handle<> value) {
  auto typedArrayHandle = Handle<JSTypedArray>::vmcast(selfHandle);
  double x;
  if (LLVM_UNLIKELY(!value->isNumber())) {
    auto res = toNumber_RJS(runtime, value);
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
      return ExecutionStatus::EXCEPTION;
    x = res->getNumber();
  } else {
    x = value->getNumber();
  }
  if (LLVM_UNLIKELY(!typedArrayHandle->attached(runtime))) {
    return runtime->raiseTypeError(
        "Cannot set a value into a detached ArrayBuffer");
  }
  if (LLVM_LIKELY(index < typedArrayHandle->getLength())) {
    typedArrayHandle->template at<T>(runtime, index) =
        JSTypedArray<T, C>::toDestType(x);
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
