/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JSARRAYBUFFER_H
#define HERMES_VM_JSARRAYBUFFER_H

#include "hermes/VM/JSObject.h"
#include "hermes/VM/Runtime.h"

namespace hermes {
namespace vm {

/// A JSArrayBuffer is a light container over an array of bytes.
///
/// This should be used in combination with a typed array view over the buffer
/// in order to extract its information in different ways.
class JSArrayBuffer final : public JSObject {
 public:
  // NOTE: This restricts the max size of an ArrayBuffer to 2 ^ 32 - 1 on 32-bit
  // platforms.
  // A RangeError for a failed allocation should be thrown if the requested
  // amount is larger than the native platform's `size_t`
  using size_type = std::size_t;

  static ObjectVTable vt;

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::ArrayBufferKind;
  }

  static PseudoHandle<JSArrayBuffer> create(
      Runtime *runtime,
      Handle<JSObject> prototype);

  /// ES7 24.1.1.4
  /// NOTE: since SharedArrayBuffer does not exist, this does not use the
  /// SpeciesConstructor, it always allocates a normal ArrayBuffer.
  static CallResult<Handle<JSArrayBuffer>> clone(
      Runtime *runtime,
      Handle<JSArrayBuffer> src,
      size_type srcByteOffset,
      size_type srcSize);

  /// ES7 6.2.6.2
  static void copyDataBlockBytes(
      JSArrayBuffer *dst,
      size_type dstIndex,
      JSArrayBuffer *src,
      size_type srcIndex,
      size_type count);

  /// Creates a data block of size \p size for this JSArrayBuffer to hold.
  /// Replaces the currently used data block.
  /// \p zero if true, zero out the data in the block, else leave it
  ///   uninitialized.
  /// \return ExecutionStatus::RETURNED iff the allocation was successful.
  ExecutionStatus
  createDataBlock(Runtime *runtime, size_type size, bool zero = true);

  /// Retrieves a pointer to the held buffer.
  /// \return A pointer to the buffer owned by this object. This can be null
  ///   if the ArrayBuffer is empty.
  /// \pre attached() must be true
  uint8_t *getDataBlock() {
    assert(attached() && "Cannot get a data block from a detached ArrayBuffer");
    return data_;
  }

  /// Get the size of this buffer.
  size_type size() const {
    return size_;
  }

  /// Whether this JSArrayBuffer is attached to some data block.
  /// NOTE: a zero size JSArrayBuffer can be attached. Make sure to check both
  /// the attached-ness and the validity of any index before using it.
  bool attached() const {
    return attached_;
  }

  /// Detaches this buffer from its data block, effectively freeing the storage
  /// and setting this ArrayBuffer to have zero size.  The \p gc argument allows
  /// the GC to be informed of this external memory deletion.
  void detach(GC *gc);

 protected:
  static void _finalizeImpl(GCCell *cell, GC *gc);
  static size_t _mallocSizeImpl(GCCell *cell);
  static gcheapsize_t _externalMemorySizeImpl(const GCCell *cell);
  static void _snapshotAddEdgesImpl(GCCell *cell, GC *gc, HeapSnapshot &snap);
  static void _snapshotAddNodesImpl(GCCell *cell, GC *gc, HeapSnapshot &snap);

 private:
  uint8_t *data_;
  size_type size_;
  bool attached_;

#ifdef HERMESVM_SERIALIZE
  explicit JSArrayBuffer(Deserializer &d);

  friend void ArrayBufferSerialize(Serializer &s, const GCCell *cell);
  friend void ArrayBufferDeserialize(Deserializer &d, CellKind kind);
#endif

  JSArrayBuffer(Runtime *runtime, JSObject *parent, HiddenClass *clazz);

  ~JSArrayBuffer() = default;
};

} // namespace vm
} // namespace hermes

#endif
