/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSArrayBuffer.h"

#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/Runtime-inline.h"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
// class JSArrayBuffer

const ObjectVTable JSArrayBuffer::vt{
    VTable(
        CellKind::JSArrayBufferKind,
        cellSize<JSArrayBuffer>(),
        _finalizeImpl,
        nullptr,
        _mallocSizeImpl,
        nullptr
#ifdef HERMES_MEMORY_INSTRUMENTATION
        ,
        VTable::HeapSnapshotMetadata {
          HeapSnapshot::NodeType::Object, nullptr, _snapshotAddEdgesImpl,
              _snapshotAddNodesImpl, nullptr
        }
#endif
        ),
    _getOwnIndexedRangeImpl,
    _haveOwnIndexedImpl,
    _getOwnIndexedPropertyFlagsImpl,
    _getOwnIndexedImpl,
    _setOwnIndexedImpl,
    _deleteOwnIndexedImpl,
    _checkAllOwnIndexedImpl,
};

void JSArrayBufferBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSArrayBuffer>());
  JSObjectBuildMeta(cell, mb);
  mb.setVTable(&JSArrayBuffer::vt);
}

PseudoHandle<JSArrayBuffer> JSArrayBuffer::create(
    Runtime &runtime,
    Handle<JSObject> parentHandle) {
  auto *cell = runtime.makeAFixed<JSArrayBuffer, HasFinalizer::Yes>(
      runtime,
      parentHandle,
      runtime.getHiddenClassForPrototype(
          *parentHandle, numOverlapSlots<JSArrayBuffer>()));
  return JSObjectInit::initToPseudoHandle(runtime, cell);
}

CallResult<Handle<JSArrayBuffer>> JSArrayBuffer::clone(
    Runtime &runtime,
    Handle<JSArrayBuffer> src,
    size_type srcOffset,
    size_type srcSize) {
  if (!src->attached()) {
    return runtime.raiseTypeError("Cannot clone from a detached buffer");
  }

  auto arr = runtime.makeHandle(JSArrayBuffer::create(
      runtime, Handle<JSObject>::vmcast(&runtime.arrayBufferPrototype)));

  // Don't need to zero out the data since we'll be copying into it immediately.
  if (createDataBlock(runtime, arr, srcSize, false) ==
      ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  if (srcSize != 0) {
    JSArrayBuffer::copyDataBlockBytes(
        runtime, *arr, 0, *src, srcOffset, srcSize);
  }
  return arr;
}

void JSArrayBuffer::copyDataBlockBytes(
    Runtime &runtime,
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
      dst->getDataBlock(runtime) != src->getDataBlock(runtime) &&
      "Cannot copy into the same block, must be different blocks");
  assert(
      srcIndex + count <= src->size() &&
      "Cannot copy more data out of a block than what exists");
  assert(
      dstIndex + count <= dst->size() &&
      "Cannot copy more data into a block than it has space for");
  // Copy from the other buffer.
  memcpy(
      dst->getDataBlock(runtime) + dstIndex,
      src->getDataBlock(runtime) + srcIndex,
      count);
}

JSArrayBuffer::JSArrayBuffer(
    Runtime &runtime,
    Handle<JSObject> parent,
    Handle<HiddenClass> clazz)
    : JSObject(runtime, *parent, *clazz), attached_(false) {}

void JSArrayBuffer::_finalizeImpl(GCCell *cell, GC &gc) {
  auto *self = vmcast<JSArrayBuffer>(cell);
  if (self->attached() && !self->external_)
    self->freeInternalBuffer(gc);
  self->~JSArrayBuffer();
}

size_t JSArrayBuffer::_mallocSizeImpl(GCCell *cell) {
  const auto *buffer = vmcast<JSArrayBuffer>(cell);
  return buffer->size_;
}

#ifdef HERMES_MEMORY_INSTRUMENTATION
void JSArrayBuffer::_snapshotAddEdgesImpl(
    GCCell *cell,
    GC &gc,
    HeapSnapshot &snap) {
  auto *const self = vmcast<JSArrayBuffer>(cell);
  if (!self->attached() || self->external_) {
    return;
  }
  if (uint8_t *data = self->data_.get(gc)) {
    // While this is an internal edge, it is to a native node which is not
    // automatically added by the metadata.
    snap.addNamedEdge(
        HeapSnapshot::EdgeType::Internal, "backingStore", gc.getNativeID(data));
    // The backing store just has numbers, so there's no edges to add here.
  }
}

void JSArrayBuffer::_snapshotAddNodesImpl(
    GCCell *cell,
    GC &gc,
    HeapSnapshot &snap) {
  auto *const self = vmcast<JSArrayBuffer>(cell);
  if (!self->attached() || self->external_) {
    return;
  }
  if (uint8_t *data = self->data_.get(gc)) {
    // Add the native node before the JSArrayBuffer node.
    snap.beginNode();
    snap.endNode(
        HeapSnapshot::NodeType::Native,
        "JSArrayBufferData",
        gc.getNativeID(data),
        self->size_,
        0);
  }
}
#endif

void JSArrayBuffer::freeInternalBuffer(GC &gc) {
  uint8_t *data = data_.get(gc);
  assert(attached() && "Buffer must be attached");
  assert((data || size_ == 0) && "Null buffers must have zero size");
  assert(!external_ && "External buffer cannot be freed");

  // Need to untrack the native memory that may have been tracked by snapshots.
  gc.debitExternalMemory(this, size_);
  gc.getIDTracker().untrackNative(data);
  free(data);
}

ExecutionStatus JSArrayBuffer::detach(
    Runtime &runtime,
    Handle<JSArrayBuffer> self) {
  if (!self->attached())
    return ExecutionStatus::RETURNED;
  if (!self->external_)
    self->freeInternalBuffer(runtime.getHeap());
  else {
    auto res = setExternalFinalizer(
        runtime, self, HandleRootOwner::getUndefinedValue());
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
      return ExecutionStatus::EXCEPTION;
  }
  // Note that whether a buffer is attached is independent of whether
  // it has allocated data.
  self->attached_ = false;
  return ExecutionStatus::RETURNED;
}

ExecutionStatus JSArrayBuffer::createDataBlock(
    Runtime &runtime,
    Handle<JSArrayBuffer> self,
    size_type size,
    bool zero) {
  if (LLVM_UNLIKELY(detach(runtime, self) == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  uint8_t *data = nullptr;
  if (size > 0) {
    // If an external allocation of this size would exceed the GC heap size,
    // raise RangeError.
    if (LLVM_UNLIKELY(!runtime.getHeap().canAllocExternalMemory(size))) {
      return runtime.raiseRangeError(
          "Cannot allocate a data block for the ArrayBuffer");
    }

    // Note that the result of calloc or malloc is immediately checked below, so
    // we don't use the checked versions.
    data = zero ? static_cast<uint8_t *>(calloc(sizeof(uint8_t), size))
                : static_cast<uint8_t *>(malloc(sizeof(uint8_t) * size));
    if (!data) {
      return runtime.raiseRangeError(
          "Cannot allocate a data block for the ArrayBuffer");
    }
  }

  self->attached_ = true;
  self->data_.set(runtime, data);
  self->size_ = size;
  self->external_ = false;
  runtime.getHeap().creditExternalMemory(*self, size);
  return ExecutionStatus::RETURNED;
}

ExecutionStatus JSArrayBuffer::setExternalFinalizer(
    Runtime &runtime,
    Handle<JSArrayBuffer> self,
    Handle<> value) {
  auto res = JSObject::defineOwnProperty(
      self,
      runtime,
      Predefined::getSymbolID(
          Predefined::InternalPropertyArrayBufferExternalFinalizer),
      DefinePropertyFlags::getDefaultNewPropertyFlags(),
      value);
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  if (LLVM_UNLIKELY(!*res))
    return runtime.raiseTypeError("Cannot modify external buffer.");
  return ExecutionStatus::RETURNED;
}

ExecutionStatus JSArrayBuffer::setExternalDataBlock(
    Runtime &runtime,
    Handle<JSArrayBuffer> self,
    uint8_t *data,
    size_type size,
    void *context,
    FinalizeNativeStatePtr finalizePtr) {
  if (LLVM_UNLIKELY(detach(runtime, self) == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;

  // Set the external finalizer first, so that if it throws, the buffer is not
  // left in an attached state.
  auto *ns = NativeState::create(runtime, context, finalizePtr);
  auto res = setExternalFinalizer(runtime, self, runtime.makeHandle(ns));
  if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;
  self->attached_ = true;
  self->size_ = size;
  self->external_ = true;
  self->data_.set(runtime, data);
  return ExecutionStatus::RETURNED;
}

} // namespace vm
} // namespace hermes
