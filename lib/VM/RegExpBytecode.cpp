/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/RegExpBytecode.h"

#include "hermes/Support/CheckedMalloc.h"

#include <cassert>
#include <cstring>
#include <limits>
#include <new>

namespace hermes {
namespace vm {

llvh::IntrusiveRefCntPtr<RegExpBytecode> RegExpBytecode::create(
    llvh::ArrayRef<uint8_t> bytecode,
    Ownership ownership) {
  assert(
      bytecode.size() >= sizeof(regex::RegexBytecodeHeader) &&
      "regexp bytecode must start with a header");
  assert(
      bytecode.size() <= std::numeric_limits<uint32_t>::max() &&
      "Bytecode size cannot exceed 32 bits");
  void *mem = checkedMalloc(bytesOffset() + bytecode.size());
  auto *self = new (mem) RegExpBytecode((uint32_t)bytecode.size(), ownership);
  memcpy(self->bytes(), bytecode.data(), bytecode.size());
  return llvh::IntrusiveRefCntPtr<RegExpBytecode>(self);
}

size_t RegExpBytecode::mallocSize() const {
  return bytesOffset() + size_;
}

} // namespace vm
} // namespace hermes
