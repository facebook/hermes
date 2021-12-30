/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_REGEXPMATCH_H
#define HERMES_VM_REGEXPMATCH_H

#include "hermes/Support/OptValue.h"

#include "llvh/ADT/SmallVector.h"

namespace hermes {
namespace vm {

/// Struct representing a single match.
struct RegExpMatchRange {
  /// Character index where the match occurred.
  uint32_t location;

  /// Number of characters of the match.
  uint32_t length;
};

/// A RegExpMatch represents a list of (sub)RegExpMatchRanges. If the range is
/// None, it means nothing matched for that submatch.
/// In a successful match, the first element is never None.
using RegExpMatch = llvh::SmallVector<OptValue<RegExpMatchRange>, 4>;

} // namespace vm
} // namespace hermes

#endif
