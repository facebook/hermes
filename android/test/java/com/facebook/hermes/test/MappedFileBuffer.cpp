/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <MappedFileBuffer.h>

#include <fcntl.h>
#include <folly/Exception.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <memory>

namespace facebook {
namespace jsi {
namespace jni {

MappedFileBuffer::MappedFileBuffer(const std::string &fileName) {
  folly::checkUnixError(fd_ = open(fileName.c_str(), O_RDONLY));
  struct stat statbuf;
  folly::checkUnixError(fstat(fd_, &statbuf));
  size_ = statbuf.st_size;
  void *bytecodeFileMap = mmap(
      /*address*/ nullptr, size_, PROT_READ, MAP_PRIVATE, fd_, /*offset*/ 0);
  assert(bytecodeFileMap != MAP_FAILED && "mmap failed.");
  data_ = reinterpret_cast<uint8_t *>(bytecodeFileMap);
}

MappedFileBuffer::~MappedFileBuffer() {
  folly::checkUnixError(munmap(data_, size_));
  folly::checkUnixError(close(fd_));
}

} // namespace jni
} // namespace jsi
} // namespace facebook
