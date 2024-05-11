/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_MEMORYBUFFER_H
#define HERMES_SUPPORT_MEMORYBUFFER_H

#include "hermes/Support/Buffer.h"
#include "llvh/Support/MemoryBuffer.h"

namespace hermes {

// Used in hvm.cpp and hermes.cpp
class MemoryBuffer : public Buffer {
 public:
  MemoryBuffer(const llvh::MemoryBuffer *buffer)
      : Buffer(
            (const uint8_t *)buffer->getBufferStart(),
            buffer->getBufferSize()) {}
};

// Like MemoryBuffer, but owns the underlying llvh::MemoryBuffer
class OwnedMemoryBuffer : public MemoryBuffer {
 public:
  OwnedMemoryBuffer(std::unique_ptr<llvh::MemoryBuffer> buffer)
      : MemoryBuffer(buffer.get()), data_(std::move(buffer)) {}

 private:
  std::unique_ptr<llvh::MemoryBuffer> data_;
};

// An adapter that wraps a hermes::Buffer in a llvh::MemoryBuffer
class HermesLLVMMemoryBuffer : public llvh::MemoryBuffer {
 public:
  HermesLLVMMemoryBuffer(
      std::unique_ptr<hermes::Buffer> buffer,
      llvh::StringRef name,
      bool requiresNullTerminator = true)
      : name_(name), data_(std::move(buffer)) {
    auto start = reinterpret_cast<const char *>(data_->data());
    auto end = start + data_->size();
    init(start, end, requiresNullTerminator);
  }
  virtual BufferKind getBufferKind() const override {
    return llvh::MemoryBuffer::BufferKind::MemoryBuffer_Malloc;
  }
  virtual llvh::StringRef getBufferIdentifier() const override {
    return name_;
  }

 private:
  std::string name_;
  std::unique_ptr<hermes::Buffer> data_;
};

/// A MemoryBuffer that owns the underlying data in a std::string.
class StdStringLLVHMemoryBuffer : public llvh::MemoryBuffer {
  std::string data_;
  std::string name_;

 public:
  StdStringLLVHMemoryBuffer(std::string &&data, std::string &&name)
      : data_(std::move(data)), name_(std::move(name)) {
    init(data_.data(), data_.data() + data_.size(), true);
  }
  BufferKind getBufferKind() const override {
    return BufferKind::MemoryBuffer_Malloc;
  }
  llvh::StringRef getBufferIdentifier() const override {
    return name_;
  }
};

} // namespace hermes

#endif // HERMES_SUPPORT_MEMORYBUFFER_H
