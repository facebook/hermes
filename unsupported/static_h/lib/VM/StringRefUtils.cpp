/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/StringRefUtils.h"

#include "hermes/Support/UTF8.h"

#include "llvh/ADT/SmallString.h"
#include "llvh/Support/ConvertUTF.h"
#include "llvh/Support/raw_ostream.h"

namespace hermes {
namespace vm {

UTF16Ref createUTF16Ref(const char16_t *str) {
  return UTF16Ref(str, utf16_traits::length(str));
}

ASCIIRef createASCIIRef(const char *str) {
  return ASCIIRef(str, ascii_traits::length(str));
}

llvh::raw_ostream &operator<<(llvh::raw_ostream &OS, ASCIIRef asciiRef) {
  return OS << llvh::StringRef(asciiRef.data(), asciiRef.size());
}

/// Print the given UTF-16. We just assume UTF-8 output to avoid the increased
/// binary size of supporting multiple encodings.
llvh::raw_ostream &operator<<(llvh::raw_ostream &OS, UTF16Ref u16ref) {
  // Note this assumes that the desired output encoding is UTF-8, which may
  // not be a valid assumption if outputting to a tty.
  std::string narrowStr;
  convertUTF16ToUTF8WithReplacements(narrowStr, u16ref);
  return OS << narrowStr;
}

} // namespace vm
} // namespace hermes
