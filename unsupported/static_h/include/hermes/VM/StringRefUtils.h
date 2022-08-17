/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_UTF16REF_H
#define HERMES_VM_UTF16REF_H

#include "llvh/ADT/ArrayRef.h"

namespace llvh {
class raw_ostream;
}

namespace hermes {
namespace vm {

using UTF16Ref = llvh::ArrayRef<char16_t>;
using UTF8Ref = llvh::ArrayRef<uint8_t>;
using ASCIIRef = llvh::ArrayRef<char>;

/// Convenient alias.
using utf16_traits = std::char_traits<char16_t>;
using ascii_traits = std::char_traits<char>;

/// Create a UTF16Ref from a zero-terminated string.
UTF16Ref createUTF16Ref(const char16_t *str);

/// Create an ASCIIRef from a zero-terminated string.
ASCIIRef createASCIIRef(const char *str);

llvh::raw_ostream &operator<<(llvh::raw_ostream &OS, ASCIIRef asciiRef);

llvh::raw_ostream &operator<<(llvh::raw_ostream &OS, UTF16Ref u16ref);

/// Check whether two ArrayRef are equal in content.
template <typename T1, typename T2>
bool stringRefEquals(llvh::ArrayRef<T1> str1, llvh::ArrayRef<T2> str2) {
  if (str1.size() != str2.size()) {
    return false;
  }
  return std::equal(str1.begin(), str1.end(), str2.begin());
}

/// Compare two ArrayRef, \return +1 if str1 > str2, -1 if str1 < str2, 0
/// otherwise.
template <typename T1, typename T2>
int stringRefCompare(llvh::ArrayRef<T1> str1, llvh::ArrayRef<T2> str2) {
  if (str1.size() >= str2.size()) {
    // If str1 is equal or longer than str2, match using str2's length.
    auto pos = std::mismatch(str2.begin(), str2.end(), str1.begin());
    // Note that pos.first is from str2, pos.second is from str1.
    if (pos.first == str2.end()) {
      // str2 reaches the end and everything is equal so far.
      if (pos.second == str1.end()) {
        // str1 also reaches the end, two strings are equal.
        return 0;
      }
      // str1 is longer, so str1 > str2.
      return +1;
    }
    // Found a different character, return based on which is bigger.
    return *pos.second > *pos.first ? +1 : -1;
  }
  // str1 is shorter than str2, match using str1's length.
  auto pos = std::mismatch(str1.begin(), str1.end(), str2.begin());
  if (pos.first == str1.end()) {
    // str1 reaches the end and everything is equal so far.
    // Since str1 is shorter than str2, str1 < str2.
    return -1;
  }
  // Found a different character, return based on which is bigger.
  return *pos.first > *pos.second ? +1 : -1;
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_UTF16REF_H
