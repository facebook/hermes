/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_BUFFER_H
#define HERMES_SUPPORT_BUFFER_H

#include <cstddef>
#include <cstdint>
#include <string>

namespace hermes {

/// A generic buffer interface.  E.g. for memmapped bytecode.
class Buffer {
 public:
  Buffer() : data_(nullptr), size_(0) {}

  Buffer(const uint8_t *data, size_t size) : data_(data), size_(size) {}

  virtual ~Buffer() {}

  const uint8_t *data() const {
    return data_;
  };

  size_t size() const {
    return size_;
  }

 protected:
  const uint8_t *data_ = nullptr;
  size_t size_ = 0;
};

/// A Buffer that calls a callback on destruction.
class CallbackBuffer : public Buffer {
 public:
  using FinalizeCb = void (*)(const uint8_t *data, size_t size, void *hint);

  CallbackBuffer(const uint8_t *data, size_t size, FinalizeCb cb, void *hint)
      : Buffer(data, size), cb_(cb), hint_(hint) {}

  ~CallbackBuffer() override {
    if (cb_)
      cb_(data_, size_, hint_);
  }

 private:
  FinalizeCb cb_;
  void *hint_;
};

/// A Buffer that owns a std::string.
class StdStringBuffer : public Buffer {
 public:
  StdStringBuffer(std::string &&str) : str_(std::move(str)) {
    data_ = reinterpret_cast<const uint8_t *>(str_.data());
    size_ = str_.size();
  }

 private:
  std::string str_;
};

} // namespace hermes

#endif
