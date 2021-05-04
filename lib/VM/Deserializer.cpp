/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMESVM_SERIALIZE
#include "hermes/ADT/CompactArray.h"

#include "hermes/VM/Deserializer.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/GCPointer.h"
#include "hermes/VM/JSArrayBuffer.h"
#include "hermes/VM/JSDataView.h"
#include "hermes/VM/JSNativeFunctions.h"
#include "hermes/VM/JSTypedArray.h"
#include "hermes/VM/JSWeakMapImpl.h"
#include "hermes/VM/PrimitiveBox.h"
#include "hermes/VM/Runtime.h"

#include "JSLib/JSLibInternal.h"

#include "llvh/Support/Debug.h"

#define DEBUG_TYPE "serialize"

namespace hermes {
namespace vm {

using DeserializeCallBack = void(Deserializer &d, CellKind kind);

static DeserializeCallBack *deserializeImpl[] = {
#define CELL_KIND(name) name##Deserialize,
#include "hermes/VM/CellKinds.def"
#undef CELL_KIND
};

void Deserializer::deserializeCell(uint8_t kind) {
  assert(
      (CellKind)kind != CellKind::ArrayStorageKind &&
      "ArrayStorage should be serialized/deserialized with its owner.");
  deserializeImpl[kind](*this, (CellKind)kind);
}

void Deserializer::deserializeCompactTable(CompactTable &table) {
  auto size = readInt<uint32_t>();
  auto scale = (CompactArray::Scale)readInt<uint8_t>();
  CompactArray tmp(size, scale);
  for (uint32_t idx = 0; idx < size; ++idx)
    tmp.set(idx, readInt<uint32_t>());
  table.asArray().swap(tmp);
}

void Deserializer::flushRelocationQueue() {
  while (!relocationQueue_.empty()) {
    auto entry = relocationQueue_.front();
    relocationQueue_.pop_front();
    assert(entry.id < objectTable_.size() && "invalid relocation id");
    void *ptr = objectTable_[entry.id];
    assert(ptr && "pointer relocation cannot be resolved");
    updateAddress(entry.address, ptr, entry.kind);
  }
}

void Deserializer::init(
    ExternalPointersVectorFunction *externalPointersVectorCallBack) {
  // Do the sanity check of the header first.
  readHeader();

  // Relocation table size and string buffers are all at the end of the
  // MemoryBuffer. Let's start reading from the back.
  const char *ptr = buffer_->getBufferEnd();

  uint32_t size;
  // Read map size and resize relocation table.
  size = readBackwards(ptr);
  objectTable_.resize(size);

  // Read size of char16Buf_.
  // Note this is the total size of the buffer and may include padding.
  size = readBackwards(ptr);
  // Move ptr to the beginning of char16Buf_.
  ptr -= size;
  if (size > 0) {
    // Has char16Buf_, reconstruct the buffer here.
    assert(ptr >= buffer_->getBufferStart() && "wrong char16Buf_ size");
    // size is buffer size in bytes. Let's calculate the end first before
    // casting to char16_t *.
    // Ensure we are aligned.
    uint32_t aligningOffset = llvh::alignTo(size, alignof(char16_t)) - size;
    assert(aligningOffset <= size && "aligning offset should not exceed size");
    assert(
        (intptr_t)(ptr + aligningOffset) % alignof(char16_t) == 0 &&
        "Pointer should be aligned for char16");
    char16Buf_ = ArrayRef<char16_t>(
        (const char16_t *)(ptr + aligningOffset),
        (const char16_t *)(ptr + size - aligningOffset));
  }

  // Read size of charBuf_.
  size = readBackwards(ptr);
  // Move ptr to the beginning of charBuf_.
  ptr -= size;
  if (size > 0) {
    // Has charBuf_, reconstruct the buffer here.
    assert(ptr >= buffer_->getBufferStart() && "wrong charBuf_ size");
    charBuf_ = ArrayRef<char>(ptr, size);
  }

  // Map nullptr to 0.
  objectTable_[0] = 0;

  // Populate relocation table for native functions and constructors.
  size_t idx = 1;
#define NATIVE_FUNCTION(func)                                                \
  assert(!objectTable_[idx]);                                                \
  objectTable_[idx] = (void *)func;                                          \
  LLVM_DEBUG(                                                                \
      llvh::dbgs() << idx << ", " << #func << ", " << (void *)func << "\n"); \
  idx++;

#define NATIVE_FUNCTION_TYPED(func, type)                           \
  assert(!objectTable_[idx]);                                       \
  objectTable_[idx] = (void *)func<type>;                           \
  LLVM_DEBUG(                                                       \
      llvh::dbgs() << idx << ", " << #func << "<" << #type << ">, " \
                   << (void *)func<type> << "\n");                  \
  idx++;

#define NATIVE_FUNCTION_TYPED_2(func, type, type2)                           \
  assert(!objectTable_[idx]);                                                \
  objectTable_[idx] = (void *)func<type, type2>;                             \
  LLVM_DEBUG(                                                                \
      llvh::dbgs() << idx << ", " << #func << "<" << #type << ", " << #type2 \
                   << ">, " << ((void *)func<type, type2>) << "\n");         \
  idx++;

  NativeConstructor::CreatorFunction *funcPtr;
#define NATIVE_CONSTRUCTOR(func)                                      \
  funcPtr = func;                                                     \
  assert(!objectTable_[idx]);                                         \
  objectTable_[idx] = (void *)funcPtr;                                \
  LLVM_DEBUG(                                                         \
      llvh::dbgs() << idx << ", " << #func << ", " << (void *)funcPtr \
                   << "\n");                                          \
  idx++;

#define NATIVE_CONSTRUCTOR_TYPED(classname, type, type2, func)         \
  funcPtr = func<classname<type, type2>>;                              \
  assert(!objectTable_[idx]);                                          \
  objectTable_[idx] = (void *)funcPtr;                                 \
  LLVM_DEBUG(                                                          \
      llvh::dbgs() << idx << ", " << #func << "<" << #classname << "<" \
                   << #type << ", " << #type2 << ">>"                  \
                   << ", " << (void *)funcPtr << "\n");                \
  idx++;
#include "hermes/VM/NativeFunctions.def"
#undef NATIVE_CONSTRUCTOR

  // Map external function pointers.
  for (auto *ptr : externalPointersVectorCallBack()) {
    assert(!objectTable_[idx] && "External pointer should only be mapped once");
    objectTable_[idx] = ptr;
    idx++;
  }
}

void Deserializer::readHeader() {
  SerializeHeader readHeader;
  readData(&readHeader, sizeof(SerializeHeader));

  if (readHeader.magic != SD_MAGIC) {
    hermes_fatal("Not a serialize file or endianness do not match");
  }
  if (readHeader.version != SD_HEADER_VERSION) {
    hermes_fatal("Serialize header versions do not match");
  }
  if (readHeader.nativeFunctionTableVersion != NATIVE_FUNCTION_VERSION) {
    hermes_fatal("Native function table versions do not match");
  }
  if (runtime_->getHeap().size() < readHeader.heapSize) {
    hermes_fatal(
        (llvh::Twine("Deserialize heap size less than Serialize heap size(") +
         llvh::StringRef(std::to_string(readHeader.heapSize)) +
         llvh::Twine(" bytes), try increase initial heap size"))
            .str());
  }

#define CHECK_HEADER_SET(header, field)                         \
  if (!header.field) {                                          \
    hermes_fatal("Serialize/Deserialize configs do not match"); \
  }

#define CHECK_HEADER_UNSET(header, field)                       \
  if (header.field) {                                           \
    hermes_fatal("Serialize/Deserialize configs do not match"); \
  }

#ifndef NDEBUG
  CHECK_HEADER_SET(readHeader, isDebug); // isDebug
#else
  CHECK_HEADER_UNSET(readHeader, isDebug);
#endif

#ifdef HERMES_ENABLE_DEBUGGER
  CHECK_HEADER_SET(readHeader, isEnableDebugger); // isEnableDebugger.
#else
  CHECK_HEADER_UNSET(readHeader, isEnableDebugger);
#endif

  runtime_->checkHeaderRuntimeConfig(readHeader);
}

void Deserializer::readAndCheckOffset() {
  size_t currentOffset = offset_;
  size_t bytes = readInt<size_t>();
  if (currentOffset != bytes) {
    hermes_fatal("Deserializer sanity check failed: offset don't match");
  }
}

void Deserializer::updateAddress(
    void *address,
    void *ptrVal,
    RelocationKind kind) {
  switch (kind) {
    case RelocationKind::NativePointer:
      *(void **)address = ptrVal;
      return;
    case RelocationKind::GCPointer:
      ((GCPointerBase *)address)
          ->set(runtime_, static_cast<GCCell *>(ptrVal), &runtime_->getHeap());
      return;
    case RelocationKind::HermesValue:
      ((HermesValue *)address)->unsafeUpdatePointer(ptrVal);
      return;
    case RelocationKind::SmallHermesValue:
      ((SmallHermesValue *)address)
          ->unsafeUpdatePointer(static_cast<GCCell *>(ptrVal), runtime_);
      return;
  }
  llvm_unreachable("Invalid relocation kind");
}

} // namespace vm
} // namespace hermes

#undef DEBUG_TYPE
#endif
