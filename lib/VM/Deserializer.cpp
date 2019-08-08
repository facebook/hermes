/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/Deserializer.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/GCPointer.h"
#include "hermes/VM/Runtime.h"

#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "serialize"

namespace hermes {
namespace vm {

void Deserializer::deserializeCell(uint8_t kind) {
  // TODO: later
}

void Deserializer::endObject(void *object) {
  uint32_t id = readInt<uint32_t>();
  assert(id < objectTable_.size() && "invalid relocation id");
  assert(
      (!objectTable_[id] || objectTable_[id] == object) &&
      "shouldn't map relocation id to different pointer values");
  objectTable_[id] = object;
  LLVM_DEBUG(llvm::dbgs() << "[endobject] " << object << ", id " << id << "\n");
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

void Deserializer::init() {
  // Do the sanity check of the header first.
  readHeader();

  // Relocation table size and string buffers are all at the end of the
  // MemoryBuffer. Let's start reading from the back.
  const char *ptr = buffer_->getBufferEnd();

  uint32_t size;
  // Read map size and resize relocation table.
  size = readBackwards(ptr);
  objectTable_.resize(size);

  // Read size of char16Buf_
  size = readBackwards(ptr);
  // Move ptr to the beginning of char16Buf_.
  ptr -= size;
  if (size > 0) {
    // Has char16Buf_, reconstruct the buffer here.
    assert(ptr >= buffer_->getBufferStart() && "wrong char16Buf_ size");
    // \p size is buffer size in bytes. Let's calculate the end first before
    // casting to char16_t *.
    char16Buf_ = ArrayRef<char16_t>(
        (const char16_t *)ptr, (const char16_t *)(ptr + size));
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
  if (runtime_->getHeap().maxSize() < readHeader.maxHeapSize) {
    hermes_fatal(
        "Deserialize heap size less than Serialize heap size, not supported");
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
}

void Deserializer::readAndCheckOffset() {
  uint32_t currentOffset = offset_;
  uint32_t bytes = readInt<uint32_t>();
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
      break;
    case RelocationKind::GCPointer:
      ((GCPointerBase *)address)->set(runtime_, ptrVal, &runtime_->getHeap());
      break;
    case RelocationKind::HermesValue:
      ((HermesValue *)address)->unsafeUpdatePointer(ptrVal);
      break;
    default:
      llvm_unreachable("Invalid relocation kind");
  }
}

} // namespace vm
} // namespace hermes
