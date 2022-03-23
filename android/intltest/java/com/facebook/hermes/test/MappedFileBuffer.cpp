/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <MappedFileBuffer.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <memory>
#include <system_error>

namespace facebook {
namespace jsi {
namespace jni {

MappedFileBuffer::MappedFileBuffer(const std::string &fileName) {
  fd_ = open(fileName.c_str(), O_RDONLY);
  if (fd_ < 0) {
    throw std::system_error(errno, std::system_category(), "open failed");
  }
  struct stat statbuf;
  if (fstat(fd_, &statbuf) < 0) {
    throw std::system_error(errno, std::system_category(), "fstat failed");
  }
  size_ = statbuf.st_size;
  void *bytecodeFileMap = mmap(
      /*address*/ nullptr, size_, PROT_READ, MAP_PRIVATE, fd_, /*offset*/ 0);
  assert(bytecodeFileMap != MAP_FAILED && "mmap failed.");
  data_ = reinterpret_cast<uint8_t *>(bytecodeFileMap);
}

MappedFileBuffer::~MappedFileBuffer() {
  if (munmap(data_, size_) < 0) {
    assert(false && "Failed to munmap MappedFileBuffer");
  }
  if (close(fd_) < 0) {
    assert(false && "Failed to close MappedFileBuffer");
  }
}

} // namespace jni
} // namespace jsi
} // namespace facebook
