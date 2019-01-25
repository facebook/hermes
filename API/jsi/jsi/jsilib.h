// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

#include <jsi/jsi.h>

namespace facebook {
namespace jsi {

class FileBuffer : public Buffer {
 public:
  FileBuffer(const std::string& path);
  ~FileBuffer();

  size_t size() const override {
    return size_;
  }

  const uint8_t* data() const override {
    return data_;
  }

 private:
  size_t size_;
  uint8_t* data_;
};

} // namespace jsi
} // namespace facebook
