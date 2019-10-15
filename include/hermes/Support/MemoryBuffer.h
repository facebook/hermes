/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_MEMORYBUFFER_H
#define HERMES_SUPPORT_MEMORYBUFFER_H

#include "hermes/Public/Buffer.h"
#include "llvm/Support/MemoryBuffer.h"

namespace hermes {

// Used in hvm.cpp and hermes.cpp
class MemoryBuffer : public Buffer {
 public:
  MemoryBuffer(const llvm::MemoryBuffer *buffer) : buffer_(buffer) {
    data_ = reinterpret_cast<const uint8_t *>(buffer_->getBufferStart());
    size_ = buffer_->getBufferSize();
  }

 private:
  const llvm::MemoryBuffer *buffer_;
};

#ifdef HERMESVM_SERIALIZE
// A Buffer which is a part of a shared MemoryBuffer. Maintains a shared_ptr to
// the MemoryBuffer that holds the data. Used by deserialziation.
class BufferFromSharedBuffer : public Buffer {
 public:
  /// \param buffer underlying MemoryBuffer that holds the data.
  BufferFromSharedBuffer(
      const uint8_t *data,
      size_t size,
      std::shared_ptr<const llvm::MemoryBuffer> buffer)
      : Buffer(data, size), buffer_(std::move(buffer)) {
    assert(
        data >= reinterpret_cast<const uint8_t *>(buffer_->getBufferStart()) &&
        (data + size <
         reinterpret_cast<const uint8_t *>(buffer_->getBufferEnd())) &&
        "Not a valid subbuffer");
  }

 private:
  std::shared_ptr<const llvm::MemoryBuffer> buffer_;
};
#endif

// Like MemoryBuffer, but owns the underlying llvm::MemoryBuffer
class OwnedMemoryBuffer : public MemoryBuffer {
 public:
  OwnedMemoryBuffer(std::unique_ptr<llvm::MemoryBuffer> buffer)
      : MemoryBuffer(buffer.get()), data_(std::move(buffer)) {}

 private:
  std::unique_ptr<llvm::MemoryBuffer> data_;
};

// An adapter that wraps a hermes::Buffer in a llvm::MemoryBuffer
class HermesLLVMMemoryBuffer : public llvm::MemoryBuffer {
 public:
  HermesLLVMMemoryBuffer(
      std::unique_ptr<hermes::Buffer> buffer,
      llvm::StringRef name,
      bool requiresNullTerminator = true)
      : name_(name), data_(std::move(buffer)) {
    auto start = reinterpret_cast<const char *>(data_->data());
    auto end = start + data_->size();
    init(start, end, requiresNullTerminator);
  }
  virtual BufferKind getBufferKind() const override {
    return llvm::MemoryBuffer::BufferKind::MemoryBuffer_Malloc;
  }
  virtual llvm::StringRef getBufferIdentifier() const override {
    return name_;
  }

 private:
  std::string name_;
  std::unique_ptr<hermes::Buffer> data_;
};

} // namespace hermes

#endif // HERMES_SUPPORT_MEMORYBUFFER_H
