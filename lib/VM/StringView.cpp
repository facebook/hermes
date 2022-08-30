/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/StringView.h"
#pragma GCC diagnostic push

#ifdef HERMES_COMPILER_SUPPORTS_WSHORTEN_64_TO_32
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif
namespace hermes {
namespace vm {

UTF16Ref StringView::getUTF16Ref(
    llvh::SmallVectorImpl<char16_t> &allocator,
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

llvh::raw_ostream &operator<<(llvh::raw_ostream &os, const StringView &sv) {
  if (sv.isASCII()) {
    return os << llvh::StringRef(sv.castToCharPtr(), sv.length());
  } else {
    return os << UTF16Ref(sv.castToChar16Ptr(), sv.length());
  }
}

} // namespace vm
} // namespace hermes
