/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/HBC/StringKind.h"

namespace hermes {

StringKind::Entry::Entry(StringKind::Kind kind, uint32_t count)
    : datum_{kind | count} {
  assert((kind & MaxCount) == 0 && "kind overlapping with count storage.");
  assert(1 <= count && count <= MaxCount && "Count out of bounds");
}

StringKind::Entry &StringKind::Entry::operator++() {
  assert(count() < MaxCount && "Count Overflow");
  datum_++;
  return *this;
}

void StringKind::Accumulator::push_back(Kind k) {
  if (LLVM_UNLIKELY(entries_.empty())) {
    entries_.emplace_back(k);
    return;
  }

  auto &back = entries_.back();
  if (back.kind() != k || LLVM_UNLIKELY(back.count() >= MaxCount)) {
    entries_.emplace_back(k);
  } else {
    ++back;
  }
}

} // namespace hermes
