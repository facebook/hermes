/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_YOUNGGENTRAITS_H
#define HERMES_VM_YOUNGGENTRAITS_H

#include "hermes/VM/SegTraits.h"

#include "llvm/ADT/iterator_range.h"

namespace hermes {
namespace vm {

class AlignedHeapSegment;
class YoungGen;

template <>
struct SegTraits<YoungGen> {
  using It = AlignedHeapSegment *;
  using Range = llvm::iterator_range<It>;
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_YOUNGGENTRAITS_H
