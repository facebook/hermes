/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// Wrapper around llvm::SmallVector<> for easier u16 string construction.
//===----------------------------------------------------------------------===//

#ifndef HERMES_VM_SMALLXSTRING_H
#define HERMES_VM_SMALLXSTRING_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"

namespace hermes {
namespace vm {

template <typename T, unsigned N>
class SmallXString : public llvm::SmallVector<T, N> {
  using Super = llvm::SmallVector<T, N>;

 public:
  /// Construct empty.
  SmallXString() {}
  /// Construct from ArrayRef.
  SmallXString(llvm::ArrayRef<T> ref) : Super(ref.begin(), ref.end()) {}
  /// Construct from zero-terminated string.
  SmallXString(const T *strz)
      : Super(strz, strz + std::char_traits<T>::length(strz)) {}
  /// Construct from StringRef.
  SmallXString(llvm::StringRef strRef)
      : Super(strRef.bytes_begin(), strRef.bytes_end()) {}
  /// Construct from a pair of iterators.
  template <typename It>
  SmallXString(It first, It end) : Super(first, end) {}

  /// Append an ArrayRef.
  SmallXString &append(llvm::ArrayRef<T> ref) {
    Super::append(ref.begin(), ref.end());
    return *this;
  }

  /// Append a zero-terminated string.
  SmallXString &append(const T *strz) {
    Super::append(strz, strz + std::char_traits<T>::length(strz));
    return *this;
  }

  /// Append a StringRef.
  SmallXString &append(llvm::StringRef strRef) {
    Super::append(strRef.bytes_begin(), strRef.bytes_end());
    return *this;
  }

  /// Append a single character.
  SmallXString &append(T ch) {
    Super::push_back(ch);
    return *this;
  }

  /// Append a single ASCII character.
  SmallXString &append(char ch) {
    Super::push_back((unsigned char)ch);
    return *this;
  }

  template <typename in_iter>
  SmallXString &append(in_iter start, in_iter end) {
    Super::append(start, end);
    return *this;
  }

  llvm::ArrayRef<T> arrayRef() const {
    return llvm::ArrayRef<T>(Super::begin(), Super::end());
  }
};

template <unsigned N>
using SmallU16String = SmallXString<char16_t, N>;

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_SMALLXSTRING_H
