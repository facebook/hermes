/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_STRINGPRIMITIVEVALUEDENSEMAPINFO_INLINE_H
#define HERMES_VM_STRINGPRIMITIVEVALUEDENSEMAPINFO_INLINE_H

#include "hermes/VM/StringPrimitiveValueDenseMapInfo.h"

#include "hermes/Support/DenseMapInfoSpecializations.h"
#include "hermes/VM/StringPrimitive.h"

namespace hermes {
namespace vm {

// For the empty and tombstone values, use the standard ones for pointers.
/*static*/
inline StringPrimitive *StringPrimitiveValueDenseMapInfo::getEmptyKey() {
  return llvh::DenseMapInfo<StringPrimitive *>::getEmptyKey();
}

/*static*/
inline StringPrimitive *StringPrimitiveValueDenseMapInfo::getTombstoneKey() {
  return llvh::DenseMapInfo<StringPrimitive *>::getTombstoneKey();
}

/*static*/
unsigned StringPrimitiveValueDenseMapInfo::getHashValue(
    const StringPrimitive *Val) {
  // The hash computation is a benevolent side effect; doesn't
  // affect abstract value.
  return const_cast<StringPrimitive *>(Val)->getOrComputeHash();
}

/*static*/
bool StringPrimitiveValueDenseMapInfo::isEqual(
    const StringPrimitive *LHS,
    const StringPrimitive *RHS) {
  // Common fast path, we hope!
  if (LHS == RHS)
    return true;

  if (LHS == getEmptyKey() || LHS == getTombstoneKey()) {
    return LHS == RHS;
  }
  // Otherwise, LHS is a real StringPrimitive.
  if (RHS == getEmptyKey() || RHS == getTombstoneKey()) {
    return false;
  }
  // Otherwise, actually compare.
  return LHS->equals(RHS);
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_STRINGPRIMITIVEVALUEDENSEMAPINFO_INLINE_H
