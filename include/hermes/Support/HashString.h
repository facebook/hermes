/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_HASHSTRING_H
#define HERMES_SUPPORT_HASHSTRING_H

#include "hermes/Support/JenkinsHash.h"

#include "llvh/ADT/ArrayRef.h"

namespace hermes {

/// Computes a hash of \p str that must be stable between compilation and
/// execution of the compiled bytecode.
/// Use the Jenkins hash for now, but that may be changed internally later.
///
/// We cannot directly call llvh::hash_value(str) because each element of
/// str can be either char or char16_t. The default iterator of
/// llvh::ArrayRef will return different types for each element, leading to
/// different hash values even when an ASCII string has the same content as
/// an UTF16 string. hashString() takes advantage of the abstraction of
/// StringView::const_iterator to always return char16_t in the iterator.
///
/// NOTE: If hashString is changed, the bytecode version must be bumped.
template <typename T>
uint32_t hashString(llvh::ArrayRef<T> str) {
  static_assert(
      sizeof(JenkinsHash) == sizeof(uint32_t), "Jenkins Hash must be 32-bit");
  hermes::JenkinsHash hash = 0;
  for (const T c : str) {
    hash = hermes::updateJenkinsHash(hash, c);
  }
  return hash;
}

namespace hash_details {
template <std::size_t Count>
constexpr uint32_t constexprHashStringHelper(const char *str) {
  return hermes::updateJenkinsHash(
      constexprHashStringHelper<Count - 1>(str), str[Count - 1]);
}

template <>
constexpr uint32_t constexprHashStringHelper<0>(const char *str) {
  return 0;
}
} // namespace hash_details

/// Return the hash of \p str, at compile time.
template <std::size_t Count>
constexpr uint32_t constexprHashString(const char (&str)[Count]) {
  // Count-1 accounts for terminating NUL.
  return hash_details::constexprHashStringHelper<Count - 1>(str);
}

} // namespace hermes

#endif
