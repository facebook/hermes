/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/JSArrayBuffer.h"

#include "hermes/VM/BuildMetadata.h"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
// class JSArrayBuffer

ObjectVTable JSArrayBuffer::vt{
    VTable(
        CellKind::ArrayBufferKind,
        sizeof(JSArrayBuffer),
        _finalizeImpl,
        nullptr,
        _mallocSizeImpl),
    _getOwnIndexedRangeImpl,
    _haveOwnIndexedImpl,
    _getOwnIndexedPropertyFlagsImpl,
    _getOwnIndexedImpl,
    _setOwnIndexedImpl,
    _deleteOwnIndexedImpl,
    _checkAllOwnIndexedImpl,
};

void ArrayBufferBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  ObjectBuildMeta(cell, mb);
}

CallResult<HermesValue> JSArrayBuffer::create(
    Runtime *runtime,
    Handle<JSObject> parentHandle) {
  void *mem = runtime->alloc</*fixedSize*/ true, HasFinalizer::Yes>(
      sizeof(JSArrayBuffer));
  auto *self = new (mem) JSArrayBuffer(
      runtime,
      *parentHandle,
      runtime->getHiddenClassForPrototypeRaw(*parentHandle));
  return HermesValue::encodeObjectValue(
      JSObject::allocateSmallPropStorage<NEEDED_PROPERTY_SLOTS>(self));
}

CallResult<Handle<JSArrayBuffer>> JSArrayBuffer::clone(
    Runtime *runtime,
    Handle<JSArrayBuffer> src,
    size_type srcOffset,
    size_type srcSize) {
  if (!src->attached()) {
    return runtime->raiseTypeError("Cannot clone from a detached buffer");
  }

  auto arrRes = JSArrayBuffer::create(
      runtime, Handle<JSObject>::vmcast(&runtime->arrayBufferPrototype));
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto arr = runtime->makeHandle<JSArrayBuffer>(*arrRes);

  // Don't need to zero out the data since we'll be copying into it immediately.
  if (arr->createDataBlock(runtime, srcSize, false) ==
      ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  if (srcSize != 0) {
    JSArrayBuffer::copyDataBlockBytes(*arr, 0, *src, srcOffset, srcSize);
  }
  return arr;
}

void JSArrayBuffer::copyDataBlockBytes(
    JSArrayBuffer *dst,
    size_type dstIndex,
    JSArrayBuffer *src,
    size_type srcIndex,
    size_type count) {
  assert(dst && src && "Must be copied between existing objects");
  if (count == 0) {
    // Don't do anything if there was no copy requested.
    return;
  }
  assert(
      dst->getDataBlock() != src->getDataBlock() &&
      "Cannot copy into the same block, must be different blocks");
  assert(
      srcIndex + count <= src->size() &&
      "Cannot copy more data out of a block than what exists");
  assert(
      dstIndex + count <= dst->size() &&
      "Cannot copy more data into a block than it has space for");
  // Copy from the other buffer.
  memcpy(dst->getDataBlock() + dstIndex, src->getDataBlock() + srcIndex, count);
}

JSArrayBuffer::JSArrayBuffer(
    Runtime *runtime,
    JSObject *parent,
    HiddenClass *clazz)
    : JSObject(runtime, &vt.base, parent, clazz),
      data_(nullptr),
      size_(0),
      attached_(false) {}

JSArrayBuffer::~JSArrayBuffer() {
  // We expect this finalizer to be called only by _finalizerImpl,
  // below.  That detaches the buffer; here we just assert that it
  // has been detached, and that resources have been deallocated.
  assert(!attached_ && !data_ && size_ == 0);
}

void JSArrayBuffer::_finalizeImpl(GCCell *cell, GC *gc) {
  auto *self = vmcast<JSArrayBuffer>(cell);
  self->detach(gc);
  self->~JSArrayBuffer();
}

size_t JSArrayBuffer::_mallocSizeImpl(GCCell *cell) {
  const auto *buffer = static_cast<JSArrayBuffer *>(cell);
  return buffer->size_;
}

void JSArrayBuffer::detach(GC *gc) {
  if (data_) {
    gc->debitExternalMemory(this, size_);
    free(data_);
    data_ = nullptr;
    size_ = 0;
  } else {
    assert(size_ == 0);
  }
  // Note that whether a buffer is attached is independent of whether
  // it has allocated data.
  attached_ = false;
}

ExecutionStatus
JSArrayBuffer::createDataBlock(Runtime *runtime, size_type size, bool zero) {
  detach(&runtime->getHeap());
  if (size == 0) {
    // Even though there is no storage allocated, the spec requires an empty
    // ArrayBuffer to still be considered as attached.
    attached_ = true;
    return ExecutionStatus::RETURNED;
  }
  // If an external allocation of this size would exceed the GC heap size,
  // raise RangeError.
  if (LLVM_UNLIKELY(!runtime->getHeap().canAllocExternalMemory(size))) {
    return runtime->raiseRangeError(
        "Cannot allocate a data block for the ArrayBuffer");
  }

  // Note that the result of calloc or malloc is immediately checked below, so
  // we don't use the checked versions.
  data_ = zero ? static_cast<uint8_t *>(calloc(sizeof(uint8_t), size))
               : static_cast<uint8_t *>(malloc(sizeof(uint8_t) * size));
  if (data_ == nullptr) {
    // Failed to allocate.
    return runtime->raiseRangeError(
        "Cannot allocate a data block for the ArrayBuffer");
  } else {
    attached_ = true;
    size_ = size;
    runtime->getHeap().creditExternalMemory(this, size);
    return ExecutionStatus::RETURNED;
  }
}

} // namespace vm
} // namespace hermes
