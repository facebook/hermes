/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSTypedArray.h"

#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/Callable.h"

#include "llvh/Support/Debug.h"
#define DEBUG_TYPE "serialize"

namespace hermes {
namespace vm {

/// @name JSTypedArrayBase
/// @{

JSTypedArrayBase::JSTypedArrayBase(
    Runtime *runtime,
    const VTable *vt,
    Handle<JSObject> parent,
    Handle<HiddenClass> clazz)
    : JSObject(runtime, vt, *parent, *clazz),
      buffer_(nullptr),
      length_(0),
      byteWidth_(0),
      offset_(0) {
  flags_.indexedStorage = true;
  flags_.fastIndexProperties = true;
}

void TypedArrayBaseBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSTypedArrayBase>());
  ObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const JSTypedArrayBase *>(cell);
  mb.addField("buffer", &self->buffer_);
}

std::pair<uint32_t, uint32_t> JSTypedArrayBase::_getOwnIndexedRangeImpl(
    JSObject *selfObj,
    Runtime *) {
  auto *self = vmcast<JSTypedArrayBase>(selfObj);
  return {0, self->getLength()};
}

#ifdef HERMESVM_SERIALIZE
JSTypedArrayBase::JSTypedArrayBase(Deserializer &d, const VTable *vt)
    : JSObject(d, vt) {
  d.readRelocation(&buffer_, RelocationKind::GCPointer);
  length_ = d.readInt<JSTypedArrayBase::size_type>();
  byteWidth_ = d.readInt<uint8_t>();
  offset_ = d.readInt<size_type>();
}

void serializeTypedArrayBase(Serializer &s, const GCCell *cell) {
  auto *self = vmcast<const JSTypedArrayBase>(cell);
  JSObject::serializeObjectImpl(
      s, cell, JSObject::numOverlapSlots<JSTypedArrayBase>());
  s.writeRelocation(self->buffer_.get(s.getRuntime()));
  s.writeInt<JSTypedArrayBase::size_type>(self->length_);
  s.writeInt<uint8_t>(self->byteWidth_);
  s.writeInt<JSTypedArrayBase::size_type>(self->offset_);
}
#endif

bool JSTypedArrayBase::_haveOwnIndexedImpl(
    JSObject *selfObj,
    Runtime *,
    uint32_t index) {
  auto *self = vmcast<JSTypedArrayBase>(selfObj);
  // Check whether the index is within the storage.
  return index < self->getLength();
}

OptValue<PropertyFlags> JSTypedArrayBase::_getOwnIndexedPropertyFlagsImpl(
    JSObject *selfObj,
    Runtime *runtime,
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
    Runtime *runtime,
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
  auto self = Handle<JSTypedArrayBase>::dyn_vmcast(thisArg);
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
      beginScaled + src->getByteOffset(),
      endScaled - beginScaled,
      src->getByteWidth());
  return Handle<JSTypedArrayBase>::vmcast(newArr);
}

ExecutionStatus JSTypedArrayBase::createBuffer(
    Runtime *runtime,
    Handle<JSTypedArrayBase> selfObj,
    size_type length) {
  assert(runtime && selfObj);

  auto tmpbuf = runtime->makeHandle(JSArrayBuffer::create(
      runtime, Handle<JSObject>::vmcast(&runtime->arrayBufferPrototype)));

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
      dstIndex * dst->getByteWidth() + dst->getByteOffset(),
      src->getBuffer(runtime),
      srcIndex * src->getByteWidth() + src->getByteOffset(),
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
  self->offset_ = offset;
  self->length_ = size / byteWidth;
}

/// @}

template <typename T, CellKind C>
JSTypedArrayBase::JSTypedArrayVTable JSTypedArray<T, C>::vt{
    {
        VTable(C, cellSize<JSTypedArray<T, C>>()),
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

#ifdef HERMESVM_SERIALIZE
template <typename T, CellKind C>
JSTypedArray<T, C>::JSTypedArray(Deserializer &d)
    : JSTypedArrayBase(d, &vt.base.base) {}

template <typename T, CellKind C>
void deserializeTypedArray(Deserializer &d, CellKind kind) {
  auto *cell = d.getRuntime()->makeAFixed<JSTypedArray<T, C>>(d);
  d.endObject(cell);
}

#define TYPED_ARRAY(name, type)                                          \
  void name##ArrayBuildMeta(const GCCell *cell, Metadata::Builder &mb) { \
    TypedArrayBaseBuildMeta(cell, mb);                                   \
    mb.setVTable(                                                        \
        &JSTypedArray<type, CellKind::name##ArrayKind>::vt.base.base);   \
  }                                                                      \
  void name##ArraySerialize(Serializer &s, const GCCell *cell) {         \
    serializeTypedArrayBase(s, cell);                                    \
    s.endObject(cell);                                                   \
  }                                                                      \
  void name##ArrayDeserialize(Deserializer &d, CellKind kind) {          \
    deserializeTypedArray<type, CellKind::name##ArrayKind>(d, kind);     \
  }
#else
#define TYPED_ARRAY(name, type)                                          \
  void name##ArrayBuildMeta(const GCCell *cell, Metadata::Builder &mb) { \
    TypedArrayBaseBuildMeta(cell, mb);                                   \
    mb.setVTable(                                                        \
        &JSTypedArray<type, CellKind::name##ArrayKind>::vt.base.base);   \
  }
#endif // HERMESVM_SERIALIZE
#include "hermes/VM/TypedArrays.def"

template <typename T, CellKind C>
CallResult<Handle<JSTypedArrayBase>> JSTypedArray<T, C>::allocate(
    Runtime *runtime,
    size_type length) {
  Handle<JSTypedArray<T, C>> ta =
      runtime->makeHandle<JSTypedArray<T, C>>(JSTypedArray<T, C>::create(
          runtime, JSTypedArray<T, C>::getPrototype(runtime)));
  if (JSTypedArrayBase::createBuffer(runtime, ta, length) ==
      ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  return Handle<JSTypedArrayBase>::vmcast(ta);
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
  auto obj = runtime->makeHandle<JSObject>(callRes->get());
  // validate that the constructed object is a TypedArray.
  if (JSTypedArrayBase::validateTypedArray(runtime, obj) ==
      ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  return Handle<JSTypedArrayBase>::vmcast(obj);
}

template <typename T, CellKind C>
PseudoHandle<JSTypedArray<T, C>> JSTypedArray<T, C>::create(
    Runtime *runtime,
    Handle<JSObject> parentHandle) {
  auto *cell = runtime->makeAFixed<JSTypedArray<T, C>>(
      runtime,
      parentHandle,
      runtime->getHiddenClassForPrototype(
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
    Handle<JSObject> parent,
    Handle<HiddenClass> clazz)
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
    return SafeNumericEncoder<T>::encode(self->at(runtime, index));
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
    typedArrayHandle->at(runtime, index) = JSTypedArray<T, C>::toDestType(x);
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

#undef DEBUG_TYPE
