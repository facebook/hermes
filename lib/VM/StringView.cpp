/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/StringView.h"

namespace hermes {
namespace vm {

UTF16Ref StringView::getUTF16Ref(
    llvm::SmallVectorImpl<char16_t> &allocator,
    bool alwaysCopy) const {
  uint32_t existingLen = allocator.size();
  if (isASCII()) {
    const char *ptr = castToCharPtr();
    allocator.append(ptr, ptr + length());
    return UTF16Ref(allocator.data() + existingLen, length());
  }
  const char16_t *ptr = castToChar16Ptr();
  if (alwaysCopy) {
    // If alwaysCopy is true, we always copy even it's already UTF16 string.
    allocator.append(ptr, ptr + length());
    return UTF16Ref(allocator.data() + existingLen, length());
  }
  return UTF16Ref(ptr, length());
}

llvm::raw_ostream &operator<<(llvm::raw_ostream &os, const StringView &sv) {
  if (sv.isASCII()) {
    return os << llvm::StringRef(sv.castToCharPtr(), sv.length());
  } else {
    return os << UTF16Ref(sv.castToChar16Ptr(), sv.length());
  }
}

} // namespace vm
} // namespace hermes
