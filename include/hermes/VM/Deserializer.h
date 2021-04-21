/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_DESERIALIZER_H
#define HERMES_VM_DESERIALIZER_H

#ifdef HERMESVM_SERIALIZE
#include "hermes/VM/GCCell.h"
#include "hermes/VM/SerializeHeader.h"
#include "hermes/VM/StringRefUtils.h"

#include "hermes/Support/MemoryBuffer.h"

using llvh::ArrayRef;
namespace hermes {

class CompactTable;

namespace vm {

class Runtime;

/// Serialization and Deserialization (S/D): The basic idea of S/D
/// VM state is to S/D a graph of entities and each entity knows how to S/D
/// itself. Pointer values (GC pointers and native pointers) will need to be
/// relocated so they can point to the correct address after objects are
/// reconstructed during deserialization. During serialization, each address
/// will be givin a relocation id when it is first seen and this relocation id
/// will be written to the serialized stream. During deserialization each
/// relocation id will be able to know its new address after the entitie is
/// reconstructed. Note that we decide to do relocation after S/D-ing an entity
/// (Serializer::endObject and Deserializer::endObject) instead of before so
/// that it's easier to handle VariableSizeRuntimeCell.
///
/// We distinguish between the following kinds of pointers: NativePointers,
/// GCPointers, pointers in HermesValue, and function pointers (used by
/// NativeFunction and NativeConstructor). Specifically for relocation, we map
/// nullptr to id 0 in both Serializer and Deserializer so that we can deal with
/// nullptr gracefully during S/D.

/// Deserializer: The Deserializer API will wrap a llvh::MemoryBuffer with a
/// running pointer (and provide unaligned reads from that pointer). We also
/// have a objectTable_ to map relocation id to the new pointer values. Whenever
/// an entity is deserialized, we will be able to update the mapping of its
/// relocation id to the new address. When a forward reference is read, the
/// address where the reference was supposed to be stored are recorded in
/// relocationQueue_. In the end, the queue is flushed to update all outstanding
/// forward references.

/// When we start deserialization, we first read the
/// relocationMap size from the end of the serialized data and resize
/// objectTable_ to that size. We then reconstruct the char* and char16_t*
/// buffers that holds the static string literals.
class Deserializer {
 public:
  /// Note: it is the caller's responsibility to make sure that MemoryBuffer
  /// can outlive the Runtime that we are deserializing.
  Deserializer(
      std::shared_ptr<llvh::MemoryBuffer> buffer,
      Runtime *runtime,
      ExternalPointersVectorFunction *externalPointersVectorCallBack)
      : runtime_(runtime), buffer_(std::move(buffer)) {
    init(externalPointersVectorCallBack);
  }

  /// Read data from the MemoryBuffer. We use memcpy for unaligned read. We also
  /// assume that S/D are performed on systems that have the same endianness.
  /// \param dst points to where to read to.
  /// \param size total bytes of data to read.
  void readData(void *dst, size_t size) {
    assert(offset_ + size >= offset_ && "Read overflowed");
    assert(
        buffer_->getBufferStart() + offset_ + size < buffer_->getBufferEnd() &&
        "Deserialize read out of range");
    memcpy(dst, buffer_->getBufferStart() + offset_, size);
    offset_ += size;
  }

  /// Read serialize data and return as an ArrayRef<T>. Read in total \p size
  /// elements of type T and increase the offset accordingly.
  template <typename T>
  ArrayRef<T> readArrayRef(size_t size) {
    assert(offset_ + size >= offset_ && "Read overflowed");
    assert(
        buffer_->getBufferStart() + offset_ + size < buffer_->getBufferEnd() &&
        "Deserialize read out of range");
    ArrayRef<T> res((const T *)buffer_->getBufferStart() + offset_, size);
    offset_ += size * sizeof(T);
    return res;
  }

  template <typename T>
  T readInt() {
    static_assert(std::is_integral<T>::value, "T must be integral for readInt");
    T res;
    readData(&res, sizeof(T));
    return res;
  }

  /// Read a string offset from MemoryBuffer. Return the address of the string
  /// literal in buffer. The strings in the buffer are not NULL-terminated. The
  /// entity that owns the string should take care of string length (e.g. S/D
  /// length first).
  const char *readCharStr() {
    size_t charOffset = readInt<uint32_t>();
    assert(charOffset < charBuf_.size() && "charBuf_ offset out of range");
    return &charBuf_[charOffset];
  }

  /// Read a string offset from MemoryBuffer. Return the address of the string
  /// literal in the char16_t buffer. The strings in the buffer are not
  /// NULL-terminated. The entity that owns the string should take care of
  /// string length (e.g. S/D length first).
  const char16_t *readChar16Str() {
    size_t char16Offset = readInt<uint32_t>();
    assert(
        char16Offset < char16Buf_.size() && "char16Buf_ offset out of range");
    return &char16Buf_[char16Offset];
  }

  size_t getOffset() const {
    return offset_;
  }

  /// Reads an id from MemoryBuffer. Update the pointer at \p address with
  /// the correct pointer value. If not yet known (forward references), record
  /// the information in the relocationQueue_ so this \p address can be
  /// updated in the end.
  void readRelocation(void *address, RelocationKind kind) {
    assert(
        kind != RelocationKind::HermesValue &&
        "use readHermesValue for HermesValue instead");
    uint32_t id = readInt<uint32_t>();
    lookupRelocation(address, id, kind);
  }

  /// Reads a HermesValue from MemoryBuffer. If it encodes a pointer, update the
  /// relocation id to the correct pointer value (if known) or record in the
  /// relocationQueue_.
  /// \p nativePointer whether this HermesValue encodes a native pointer
  /// points to entities on the C heap. We need to provide this hint here
  /// otherwise we wouldn't be able to distinguish between a native pointer and
  /// other native values.
  void readHermesValue(HermesValue *hv, bool nativePointer = false) {
    readData(hv, sizeof(HermesValue));
    if (!hv->isPointer() && !nativePointer) {
      return;
    }
    // Extract relocation id from HermesValue and see if we can do relocation
    // now.
    uint32_t relo_id = getHermesValueRelocationID(*hv);
    lookupRelocation(hv, relo_id, RelocationKind::HermesValue);
  }

  /// Reads a SmallHermesValue from MemoryBuffer. If it encodes a pointer,
  /// update the relocation id to the correct pointer value (if known) or record
  /// in the relocationQueue_.
  void readSmallHermesValue(SmallHermesValue *shv) {
    readData(shv, sizeof(SmallHermesValue));
    if (!shv->isPointer()) {
      return;
    }
    // Extract relocation id from HermesValue and see if we can do relocation
    // now.
    uint32_t relo_id = shv->getRelocationID();
    lookupRelocation(shv, relo_id, RelocationKind::SmallHermesValue);
  }

  /// Called at the end of deserialization. Because every entity has been
  /// deserialized at this point, we should be able to update prior forward
  /// references that we were not able to update before.
  void flushRelocationQueue();

  /// Read a relocation id from the stream and set objectTable_[id] = object.
  /// Do this after object creation instead of before because there may
  /// be some variable length objects.
  /// \param object The object to end and relocate.
  void endObject(void *object) {
    uint32_t id = readInt<uint32_t>();
    assert(id < objectTable_.size() && "invalid relocation id");
    assert(
        (!objectTable_[id] || objectTable_[id] == object) &&
        "shouldn't map relocation id to different pointer values");
    objectTable_[id] = object;
  }

  Runtime *getRuntime() {
    return runtime_;
  }

  /// Return the new pointer if \p id is already be materialized, otherwise
  /// return nullptr.
  void *ptrRelocationOrNull(uint32_t id) {
    assert(id < objectTable_.size() && "invalid relocation id");
    return objectTable_[id];
  }

  /// Return the new pointer of the next id in the serialized data. Assert the
  /// id must have been materialized already.
  void *getNonNullPtr() {
    uint32_t id = readInt<uint32_t>();
    assert(id < objectTable_.size() && "invalid relocation id");
    void *ptr = objectTable_[id];
    assert(ptr && "ptr is not non-null");
    return ptr;
  }

  /// Deserialize a GCCell which has CellKind \p kind.
  void deserializeCell(uint8_t kind);

  /// Deserialize a CompactTable (replacing any previous contents).
  void deserializeCompactTable(CompactTable &table);

  /// Read serialize data header and check to see if we have the same configs as
  /// the serializing machine.
  void readHeader();

  /// Deserialize an offset from the MemoryBuffer. This offset represents how
  /// many bytes we should have read at this time from the stream. Compare with
  /// curret offset to see if we are in sync with Serialzier until this point.
  void readAndCheckOffset();

  /// Return the next \p size bytes as an unique_ptr<const Buffer>. Increase
  /// offset by \p size.
  std::unique_ptr<const BufferFromSharedBuffer> readBuffer(size_t size) {
    assert(offset_ + size >= offset_ && "Read overflowed");
    assert(
        buffer_->getBufferStart() + offset_ + size < buffer_->getBufferEnd() &&
        "Deserialize read out of range");
    auto resPtr = std::make_unique<const BufferFromSharedBuffer>(
        reinterpret_cast<const uint8_t *>(buffer_->getBufferStart()) + offset_,
        size,
        buffer_);
    offset_ += size;
    return resPtr;
  }

  /// Align offset_ with \p alignment.
  /// Default alignment set to the same as in BytecodeDataProvider.cpp.
  void align(uint32_t alignment = hbc::BYTECODE_ALIGNMENT) {
    // Support alignment as many as 8 bytes.
    assert(
        alignment > 0 && alignment <= 8 &&
        ((alignment & (alignment - 1)) == 0) && "Unsupported alignment.");
    if (offset_ % alignment == 0)
      return;
    offset_ += alignment - offset_ % alignment;
  }

 private:
  /// Extract relocation id from a read serialized HermesValue.
  uint32_t getHermesValueRelocationID(HermesValue &hv) {
    return (uint32_t)hv.getRaw();
  }

  /// Initialize data structures for deserialization. Read map size and resize
  /// relocation map. Reconstruct string literal buffers.
  void init(ExternalPointersVectorFunction *externalPointersVectorCallBack);

  /// Check if an id is materialized. If so update value at \p address
  /// according to \p kind. Otherwise recode in relocationQueue_.
  void lookupRelocation(void *address, uint32_t id, RelocationKind kind) {
    assert(id < objectTable_.size() && "relocation id out of bound");
    if (id == 0 || objectTable_[id]) {
      updateAddress(address, objectTable_[id], kind);
    } else {
      relocationQueue_.push_back({address, id, kind});
    }
  }

  /// Update pointer value at \p address to \p ptrVal according to
  /// \p kind.
  void updateAddress(void *address, void *ptrVal, RelocationKind kind);

  /// Helper function to read from the end of the memory buffer.
  /// Moves \ptr towards the beginning of the MemoryBuffer \p sizeof(uint32_t)
  /// bytes and read a uint32_t. Performs necessary bounds check.
  uint32_t readBackwards(const char *&ptr) {
    ptr -= sizeof(uint32_t);
    assert(
        ptr >= buffer_->getBufferStart() && ptr < buffer_->getBufferEnd() &&
        "read out of bounds");
    uint32_t res;
    memcpy(&res, ptr, sizeof(uint32_t));
    return res;
  }

  /// Data structure to record forward reference that needs to be updated in the
  /// end.
  struct RelocationEntry {
    void *address;
    uint32_t id;
    RelocationKind kind;
  };

  /// Pointer the the Runtime that we are deserializing. Also used as a
  /// PointerBase for GCPointer.
  Runtime *runtime_;

  /// A queue to remember forward references we see during deserialization.
  std::deque<RelocationEntry> relocationQueue_;

  /// A MemoryBuffer that holds the serialized data and where we read from.
  std::shared_ptr<const llvh::MemoryBuffer> buffer_;

  // Current offset to read from the buffer.
  size_t offset_{0};

  // Acts as a relocation table, maps relocation id to the new address. Vector
  // index is the relocation id and value is either nullptr (not materialized
  // yet) or the new pointer value. Resized when deserialization starts.
  std::vector<void *> objectTable_;

  /// Points to the char* buffer that holds all the char * string literals.
  /// Reconstructed from serialized data at the beginning of deserialization.
  ArrayRef<char> charBuf_{};

  /// Points to the char16_t* buffer that holds all the char16_t * string
  /// literals. Reconstructed from serialized data at the beginning of
  /// deserialization.
  ArrayRef<char16_t> char16Buf_{};
};

/// Forward declare all Deserialize functions.
/// These must be defined in other files.
#define CELL_KIND(name) void name##Deserialize(Deserializer &d, CellKind kind);
#include "hermes/VM/CellKinds.def"
#undef CELL_KIND

} // namespace vm
} // namespace hermes

#endif // HERMESVM_SERIALIZE
#endif
