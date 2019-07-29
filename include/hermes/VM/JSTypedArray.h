/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_JSTYPEDARRAY_H
#define HERMES_VM_JSTYPEDARRAY_H

#include "hermes/VM/GCPointer.h"
#include "hermes/VM/JSArrayBuffer.h"

namespace hermes {
namespace vm {

/// JSTypedArrayBase is the object that all JSTypedArrays inherit from, and
/// exposes common interface elements to JSTypedArrays.
class JSTypedArrayBase : public JSObject {
 public:
  using size_type = std::size_t;
  using Super = JSObject;

  struct JSTypedArrayVTable {
    ObjectVTable base;

    /// Allocate a new instance of a TypedArray matching the runtime type of
    /// the current TypedArray.
    /// \p length the length of the TypedArray to create.
    CallResult<Handle<JSTypedArrayBase>> (
        *allocate)(Runtime *runtime, size_type length);

    /// Allocate a new instance of a TypedArray from the species constructor
    /// of the given \p self, and forward the \p length parameter to that
    /// constructor.
    CallResult<Handle<JSTypedArrayBase>> (*allocateSpecies)(
        Handle<JSTypedArrayBase> self,
        Runtime *runtime,
        size_type length);
  };

  const JSTypedArrayVTable *getVT() const {
    return reinterpret_cast<const JSTypedArrayVTable *>(GCCell::getVT());
  }

  static CallResult<Handle<JSTypedArrayBase>> allocate(
      Handle<JSTypedArrayBase> self,
      Runtime *runtime,
      size_type length = 0) {
    return self->getVT()->allocate(runtime, length);
  }

  /// Allocate a new instance of a TypedArray matching the runtime type of
  /// the \p src TypedArray.
  /// The newly allocated TypedArray shares the same underlying ArrayBuffer as
  /// \p src.
  /// \p beginIndex the first index of \p src to use in the new array.
  /// \p endIndex the last index of \p src to use in the new array.
  static CallResult<Handle<JSTypedArrayBase>> allocateToSameBuffer(
      Runtime *runtime,
      Handle<JSTypedArrayBase> src,
      size_type beginIndex,
      size_type endIndex);

  static CallResult<Handle<JSTypedArrayBase>> allocateSpecies(
      Runtime *runtime,
      Handle<JSTypedArrayBase> self,
      size_type length) {
    return self->getVT()->allocateSpecies(self, runtime, length);
  }

  /// ES6 22.2.3.5.1
  /// Validates \p thisArg to be a JSTypedArrayBase.
  /// If \p checkAttached is true, it will also ensure that the typed array is
  /// attached.
  static ExecutionStatus validateTypedArray(
      Runtime *runtime,
      Handle<> thisArg,
      bool checkAttached = true);
  static bool classof(const GCCell *cell) {
    return kindInRange(
        cell->getKind(),
        CellKind::TypedArrayBaseKind_first,
        CellKind::TypedArrayBaseKind_last);
  }

  /// Get the length of this array. This corresponds to the property visible
  /// as `.length` in JS.
  size_type getLength() const {
    return length_;
  }

  /// Get the byte length of this array. This corresponds to the property
  /// visible as `.byteLength` in JS.
  JSArrayBuffer::size_type getByteLength() const {
    return length_ * byteWidth_;
  }

  /// Get the width of the type in number of bytes. This is sizeof(T) for the
  /// T used to instantiate a TypedArray.
  uint8_t getByteWidth() const {
    assert(byteWidth_ != 0 && "Cannot have a byte width of 0");
    return byteWidth_;
  }

  /// \return The underlying array buffer that this TypedArray uses for its
  /// storage.
  /// \pre This cannot be called on a detached TypedArray.
  JSArrayBuffer *getBuffer(Runtime *runtime) const {
    assert(buffer_ && "Must have some JSArrayBuffer");
    return buffer_.get(runtime);
  }

  uint8_t *begin() {
    assert(src_ && "Cannot get a nullptr");
    return src_;
  }
  uint8_t *end() {
    assert(src_ && "Cannot get a nullptr");
    return src_ + getByteLength();
  }

  /// \return Whether this JSTypedArrayBase is attached to some buffer.
  bool attached(Runtime *runtime) const {
    return buffer_ && buffer_.get(runtime)->attached();
  }

  /// \return The offset from the beginning of the buffer this typed array
  /// is looking into.
  JSArrayBuffer::size_type getByteOffset(Runtime *runtime) const {
    if (!src_ || !buffer_) {
      // For a non-attached TypedArray, report its byte offset as 0.
      return 0;
    }
    return src_ - buffer_.get(runtime)->getDataBlock();
  }

  /// Retrieve the \p i'th element of the buffer.
  /// \pre
  ///   This cannot be called on a detached TypedArray.
  ///   i must be less than the length of the TypedArray.
  template <typename T>
  T &at(Runtime *runtime, size_type i) const {
    assert(attached(runtime) && "at() requires a JSArrayBuffer");
    assert(i < getLength() && "That index is out of bounds of this TypedArray");
    return *(reinterpret_cast<T *>(src_) + i);
  }

  /// Allocates a buffer using \p runtime with \p length number of
  /// elements, each of \p byteWidth size in bytes.
  static ExecutionStatus createBuffer(
      Runtime *runtime,
      Handle<JSTypedArrayBase> selfObj,
      size_type length);

  /// Sets the current buffer to a copy of \p src starting from
  /// \p byteOffset and going for \p srcSize bytes total.
  /// \pre
  ///   src must not be a null handle.
  ///   byteOffset + srcSize <= src->size()
  static ExecutionStatus setToCopyOfBuffer(
      Runtime *runtime,
      Handle<JSTypedArrayBase> dst,
      JSArrayBuffer::size_type dstByteOffset,
      Handle<JSArrayBuffer> src,
      JSArrayBuffer::size_type srcByteOffset,
      JSArrayBuffer::size_type count);

  /// Sets the current buffer to a copy of \p src.
  /// This is a convenient wrapper around `setToCopyOfBuffer` for when a
  /// whole TypedArray must be copied into the current one.
  /// \pre src must not be a null handle.
  static ExecutionStatus setToCopyOfTypedArray(
      Runtime *runtime,
      Handle<JSTypedArrayBase> dst,
      size_type dstIndex,
      Handle<JSTypedArrayBase> src,
      size_type srcIndex,
      size_type count);

  /// Sets this object to use \p buf as its buffer, starting from \p offset
  /// position and goes for \p size indices in the buffer, where each element
  /// is aligned on \p byteWidth.  The \p gc parameter is necessary for write
  /// barriers.
  /// \pre
  ///   Both \p offset and \p size must be aligned on byteWidth.
  ///   \p offset + size <= the size of \p buf.
  ///   Neither \p self nor \p buf can be null.
  static void setBuffer(
      Runtime *runtime,
      JSTypedArrayBase *self,
      JSArrayBuffer *buf,
      JSArrayBuffer::size_type offset,
      JSArrayBuffer::size_type size,
      uint8_t byteWidth);

 protected:
  /// buffer_ is the underlying buffer which holds the data to be viewed.
  /// This buffer may be shared with other JSTypedArray instantiations.
  GCPointer<JSArrayBuffer> buffer_;
  /// length_ is the length of the buffer in terms of the type it is viewed by.
  size_type length_;
  /// byteWidth_ is the number of bytes a specific type needs to be stored.
  /// It is sizeof(Typename).
  uint8_t byteWidth_;
  /// src_ is the pointer pointing directly into the buffer to be read from.
  uint8_t *src_;

  explicit JSTypedArrayBase(
      Runtime *runtime,
      const VTable *vt,
      JSObject *parent,
      HiddenClass *clazz);

  /// Sets the current buffer's contents to the contents of a buffer from
  /// another TypedArray \p src.
  /// Copies from \p srcIndex to \p srcIndex + \p count in \p src into
  /// \p dstIndex to \p dstIndex + \p count in \p dst.
  /// \pre
  ///   Both dst and src must be the same runtime type of TypedArray.
  ///   For distinct TypedArrays, use `setToCopyOfTypedArray` instead.
  static void setToCopyOfBytes(
      Runtime *runtime,
      Handle<JSTypedArrayBase> dst,
      size_type dstIndex,
      Handle<JSTypedArrayBase> src,
      size_type srcIndex,
      size_type count);

  static std::pair<uint32_t, uint32_t> _getOwnIndexedRangeImpl(
      JSObject *selfObj,
      Runtime *runtime);

  static bool
  _haveOwnIndexedImpl(JSObject *selfObj, Runtime *runtime, uint32_t index);
  static OptValue<PropertyFlags> _getOwnIndexedPropertyFlagsImpl(
      JSObject *selfObj,
      Runtime *runtime,
      uint32_t index);

  /// Check whether all indexed properties satisfy the requirement specified by
  /// \p mode. Either whether they are all non-configurable, or whether they are
  /// all both non-configurable and non-writable.
  static bool _checkAllOwnIndexedImpl(
      JSObject *selfObj,
      Runtime *runtime,
      ObjectVTable::CheckAllOwnIndexedMode mode);

  friend void TypedArrayBaseBuildMeta(
      const GCCell *cell,
      Metadata::Builder &mb);
};

/// JSTypedArray is a collection with array semantics (random access indexing),
/// but stores a typed value with a known size, and has a static total size.
///
/// The templated versions represent the TypedArrays family from JS,
/// for example Int8Array is JSTypedArray<int8_t, CellKind::Int8ArrayKind>
template <typename T, CellKind C>
class JSTypedArray final : public JSTypedArrayBase {
 public:
  using iterator = T *;

  static JSTypedArrayVTable vt;

  static bool classof(const GCCell *cell) {
    return cell->getKind() == C;
  }

  static CallResult<HermesValue> create(
      Runtime *runtime,
      Handle<JSObject> prototype);

  iterator begin() {
    return reinterpret_cast<T *>(src_);
  };
  iterator end() {
    return reinterpret_cast<T *>(src_) + length_;
  };

  static Handle<JSObject> getPrototype(const Runtime *runtime);
  static Handle<Callable> getConstructor(const Runtime *runtime);
  static SymbolID getName(Runtime *runtime);

  /// The same as allocate is defined in the VTable, but callable directly from
  /// the class template rather than dynamically.
  static CallResult<Handle<JSTypedArrayBase>> allocate(
      Runtime *runtime,
      size_type length = 0);

  /// Converts a \p value to the type used by this typed array.
  /// NOTE: this function has specializations for types which don't use a
  /// truncated int32 representation. See the bottom of this file for their
  /// implementations.
  static inline T toDestType(double number) {
    return hermes::truncateToInt32(number);
  }

 protected:
  /// See JSTypedArrayVTable for explanations of these functions.
  static CallResult<Handle<JSTypedArrayBase>> _allocateSpeciesImpl(
      Handle<JSTypedArrayBase> self,
      Runtime *runtime,
      size_type length);
  /// Retrieve an indexed property.
  static HermesValue
  _getOwnIndexedImpl(JSObject *self, Runtime *runtime, uint32_t index);
  static CallResult<bool> _setOwnIndexedImpl(
      Handle<JSObject> selfHandle,
      Runtime *runtime,
      uint32_t index,
      Handle<> value);

 private:
  explicit JSTypedArray(Runtime *runtime, JSObject *parent, HiddenClass *clazz);
};

/// @name toDestType specializations
/// @{

template <>
inline uint8_t
JSTypedArray<uint8_t, CellKind::Uint8ClampedArrayKind>::toDestType(
    double number) {
  return toUInt8Clamp(number);
}

template <>
inline float JSTypedArray<float, CellKind::Float32ArrayKind>::toDestType(
    double number) LLVM_NO_SANITIZE("float-cast-overflow");

template <>
inline float JSTypedArray<float, CellKind::Float32ArrayKind>::toDestType(
    double number) {
  // This can overflow a float, but float overflow goes to Infinity
  // (the correct behavior) on all modern platforms.
  return number;
}

template <>
inline double JSTypedArray<double, CellKind::Float64ArrayKind>::toDestType(
    double number) {
  return number;
}

/// @}
#define TYPED_ARRAY(name, type) \
  using name##Array = JSTypedArray<type, CellKind::name##ArrayKind>;
#include "hermes/VM/TypedArrays.def"

} // namespace vm
} // namespace hermes

#endif
