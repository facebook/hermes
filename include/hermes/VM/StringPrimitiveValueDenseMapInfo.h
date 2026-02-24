/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_STRINGPRIMITIVEVALUEDENSEMAPINFO_H
#define HERMES_VM_STRINGPRIMITIVEVALUEDENSEMAPINFO_H

#include "hermes/Support/DenseMapInfoSpecializations.h"

namespace hermes {
namespace vm {

/// This can be used as the KeyInfoT argument for a DenseMap whose
/// KeyT is StringPrimitive *.  By default, a table with this key type
/// would treat it the key type just as a pointer -- different pointer
/// values would be considered distinct values.  This KeyInfoT changes
/// that to look at the values of the strings, for equality and
/// hashing.
struct StringPrimitiveValueDenseMapInfo {
  static inline StringPrimitive *getEmptyKey();

  static inline StringPrimitive *getTombstoneKey();

  static inline unsigned getHashValue(const StringPrimitive *Val);

  static inline bool isEqual(
      const StringPrimitive *LHS,
      const StringPrimitive *RHS);
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_STRINGPRIMITIVEVALUEDENSEMAPINFO_H
