/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_SERIALIZER_H
#define HERMES_VM_SERIALIZER_H

#ifdef HERMESVM_SERIALIZE
#include "hermes/VM/GCCell.h"
#include "hermes/VM/SerializeHeader.h"
#include "hermes/VM/StringRefUtils.h"

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

/// Serializer: The Serializer API will wrap a llvh::raw_ostream. It has a
/// relocation map that maps each unique pointer value it has seen to an unique
/// relocation id. When serializing an entity, ordinary data (numbers etc.
/// except string literals) are binary copied to the stream and pointer values
/// will be givin a relocation id to write to the stream in place of the
/// pointer. String literals (static char * or char16_t *) are stored in either
/// charBuf_ or char16Buf_ when they are encountered and the offset to the
/// beginning of the buffer will be written to the stream.
///
/// At the end of serialization, character buffers will be flushed to the end of
/// the stream as well as the total size of the relocation map. Therefore, the
/// serialized data will have the following sections after serialization.
/// ========================
/// SerializeHeader
/// ========================
/// Serialized Entity Data
/// ========================
/// charBuf_(used to deserilize static char* )
/// ========================
/// charBufSize(size of charBuf_ in bytes)
/// ========================
/// char16Buf_(used to deserilize static char16_t* )
/// ========================
/// char16BufSize(size of char16Buf_ in bytes)
/// ========================
/// mapSize
///
/// We assume the serializing system and the deserializing system are
/// compatible, e.g. have the same endianness and the same compile flags. To
/// perform this check, we check SerializeHeader before deserializing anything.
class Serializer {
 public:
  Serializer(
      llvh::raw_ostream &OS,
      Runtime *runtime,
      ExternalPointersVectorFunction *externalPointersVectorCallBack);

  Runtime *getRuntime() {
    return runtime_;
  }

  template <typename T>
  void writeInt(T value) {
    static_assert(
        std::is_integral<T>::value, "T must be integral for writeInt");
    os_.write(reinterpret_cast<const char *>(&value), sizeof(T));
    writtenBytes_ += sizeof(T);
  }

  /// Binary copy date to the stream.
  /// \param addr the starting addr of the data to write.
  /// \param size total number of bytes to write.
  void writeData(const void *addr, size_t size) {
    os_.write(static_cast<const char *>(addr), size);
    writtenBytes_ += size;
  }

  /// Store an ASCII string literal to charBuf_. Output the starting offset to
  /// the stream.
  void writeCharStr(ASCIIRef str) {
    charBuf_.insert(charBuf_.end(), str.begin(), str.end());
    writeInt<uint32_t>(charBufOffset_);
    charBufOffset_ += str.size();
  }

  /// Store an UTF16 string literal to char16Buf_. Output the starting offset to
  /// the stream.
  void writeChar16Str(UTF16Ref str) {
    char16Buf_.insert(char16Buf_.end(), str.begin(), str.end());
    writeInt<uint32_t>(char16BufOffset_);
    char16BufOffset_ += str.size();
  }

  /// Write a HermesValue to the stream. If it's a pointer, replace the pointer
  /// to a relocation ID \p hv and write it to the stream.
  /// Otherwise write \p hv directly. Pointers to objects on the C heap will
  /// be encoded as NativeValue, in which case hv.isPointer() will be false. Use
  /// \p nativePointer to specify if this is the case and if we need to
  /// interpret the NativeValue of \p hv as a pointer.
  void writeHermesValue(HermesValue hv, bool nativePointer = false) {
    if (hv.isPointer()) {
      void *pointer = hv.getPointer();
      uint32_t id = lookupObject(pointer);
      // Replace the pointer with the ID.
      HermesValue updated_hv = updateRelocationID(hv, id);
      writeData(&updated_hv, sizeof(updated_hv));
    } else if (nativePointer) {
      assert(hv.isNativeValue() && "must be a native value");
      void *pointer = hv.getNativePointer<void>();
      uint32_t id = lookupObject(pointer);
      HermesValue updated_hv = updateRelocationID(hv, id);
      writeData(&updated_hv, sizeof(updated_hv));
    } else {
      writeData(&hv, sizeof(hv));
    }
  }

  /// Write a SmallHermesValue to the stream. If it's a pointer, replace the
  /// pointer to a relocation ID and write it to the stream. Otherwise
  /// write \p shv directly.
  void writeSmallHermesValue(SmallHermesValue shv);

  size_t getOffset() const {
    return writtenBytes_;
  }

  /// Check if \p ptr already has a relocation id. If not assign a new id.
  /// Output the id to the stream.
  /// Called when serialize a pointer value.
  uint32_t writeRelocation(const void *ptr) {
    uint32_t id = lookupObject(ptr);
    writeInt<uint32_t>(id);
    return id;
  }

  /// Called after serializng any object. Calls \p writeReloation to get/ assign
  /// an ID for \p object and writes the id to the stream.
  uint32_t endObject(const void *object) {
    uint32_t res = writeRelocation(object);
    return res;
  }

  /// Called at the end of serialization. Write string buffers to the stream as
  /// well as totoal relocation map size.
  void writeEpilogue() {
    flushCharBufs();
    writeInt<uint32_t>(relocationMap_.size());
  }

  /// Serialize a GCCell \p on the heap.
  void serializeCell(const GCCell *cell);

  /// Serialize a CompactTable.
  void serializeCompactTable(CompactTable &table);

  /// Write serialize header so deserialize can sanity check.
  void writeHeader();

  /// Write how many bytes we have written at this point. Called when we begin
  /// to serialize each section. Will be read by deserializer for sanity check.
  void writeCurrentOffset();

  /// \return whether \p object already have a relocation id.
  bool objectInTable(void *object) {
    return relocationMap_.count(object) > 0;
  }

  /// Padding the binary according to the \p alignment.
  void pad(uint32_t alignment = hbc::BYTECODE_ALIGNMENT) {
    // Support alignment as many as 8 bytes.
    assert(
        alignment > 0 && alignment <= 8 &&
        ((alignment & (alignment - 1)) == 0) && "Unsupported alignment.");
    if (writtenBytes_ % alignment == 0)
      return;
    while (writtenBytes_ % alignment) {
      writeInt<uint8_t>(0);
    }
  }

 private:
  /// Check if the pointer value has been seen and already assigned a relocation
  /// id. If not, assign a new id. Return the id.
  uint32_t lookupObject(const void *object) {
    if (relocationMap_.count(object) > 0) {
      // Already in the map, return the relocation id.
      return relocationMap_[object];
    }

    relocationMap_[object] = currentId_;
    return currentId_++;
  }

  /// Replace pointer in \p hv with the relocation id.
  HermesValue updateRelocationID(HermesValue hv, uint32_t id) {
    hv.unsafeUpdatePointer(
        reinterpret_cast<void *>(static_cast<uintptr_t>(id)));
    return hv;
  }

  SmallHermesValue updateRelocationID(SmallHermesValue shv, uint32_t id) {
    shv.unsafeUpdateRelocationID(id);
    return shv;
  }

  /// Write string buffers and their sizes to the stream.
  void flushCharBufs();

  /// Output Stream.
  llvh::raw_ostream &os_;

  /// Total bytes written so far.
  size_t writtenBytes_{0};

  /// The next relocation id to be assigned to a new pointer value.
  /// We always map nullptr to 0. So for id for other pointer values will start
  /// at 1.
  uint32_t currentId_{1};

  /// Current relocation map. Maps each unique pointer value to an unique
  /// relocation id.
  llvh::DenseMap<const void *, uint32_t> relocationMap_;

  /// Pointer to the Runtime that the Serializer is used. Also used as
  /// PointerBase for GCPointer.
  Runtime *runtime_;

  /// Buffer to store static char* string literals during serialization, will be
  /// flushed to the end of the serialized stream.
  std::vector<char> charBuf_;

  /// Index into charBuf_ for the next string to be put into charBuf_.
  uint32_t charBufOffset_{0};

  /// Buffer to store static char16_t* string literals during serialization,
  /// will be flushed to the end of the serialized stream.
  std::vector<char16_t> char16Buf_;

  /// Index into char16Buf_ for the next string to be put into char16Buf_.
  uint32_t char16BufOffset_{0};
};

/// Forward declare all Serialize functions.
/// These must be defined in other files.
#define CELL_KIND(name) void name##Serialize(Serializer &s, const GCCell *cell);
#include "hermes/VM/CellKinds.def"
#undef CELL_KIND

} // namespace vm
} // namespace hermes

#endif // HERMESVM_SERIALIZE
#endif
