/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifdef HERMESVM_PROFILER_BB
#ifndef INLINECACHE_PROFILER_H
#include "hermes/VM/Profiler/InlineCacheProfiler.h"
#include "hermes/VM/CodeBlock.h"
#include "hermes/VM/Handle.h"
#include "hermes/VM/HiddenClass.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StringView.h"
#include "hermes/VM/SymbolID.h"

#include <vector>

namespace hermes {
namespace vm {

bool InlineCacheProfiler::insertICMiss(
    CodeBlock *codeblock,
    uint32_t instOffset,
    SymbolID &propertyID,
    ClassId objectHiddenClassId,
    ClassId cachedHiddenClassId) {
  // if not exist, create inline caching entry for the source location
  ICSrcKey icKey(instOffset, codeblock);
  auto it = cacheMisses_.find(icKey);
  if (it == cacheMisses_.end()) {
    ICMiss icMiss;
    auto p = cacheMisses_.try_emplace(icKey, icMiss);
    assert(p.second && "Guaranteed to insert");
    it = p.first;
  }
  // record the hidden class pair for the source location
  auto hcPair =
      std::pair<ClassId, ClassId>(objectHiddenClassId, cachedHiddenClassId);
  auto icRecord =
      std::pair<PropertyId, HiddenClassPair>(propertyID.unsafeGetRaw(), hcPair);
  it->second.insertMiss(icRecord);

  ++totalMisses_;
  return true;
}

JSArray *InlineCacheProfiler::getHiddenClassArray() {
  return cachedHiddenClassesRawPtr_;
}

void InlineCacheProfiler::setHiddenClassArray(JSArray *hiddenClassArray) {
  cachedHiddenClassesRawPtr_ = hiddenClassArray;
}

uint32_t &InlineCacheProfiler::getHiddenClassArrayIndex() {
  return hcIdx_;
}

llvm::DenseMap<InlineCacheProfiler::ClassId, int32_t>
    &InlineCacheProfiler::getClassIdtoIndexMap() {
  return classIdToIdx_;
}
} // namespace vm
} // namespace hermes
#endif
#endif
