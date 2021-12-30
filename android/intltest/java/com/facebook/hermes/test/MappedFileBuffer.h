/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <jsi/jsi.h>

namespace facebook {
namespace jsi {
namespace jni {

class MappedFileBuffer : public jsi::Buffer {
 public:
  explicit MappedFileBuffer(const std::string &fileName);

  size_t size() const override {
    return size_;
  }

  const uint8_t *data() const override {
    return data_;
  }

  ~MappedFileBuffer() override;

 private:
  int fd_;
  uint8_t *data_;
  size_t size_;
};

} // namespace jni
} // namespace jsi
} // namespace facebook
