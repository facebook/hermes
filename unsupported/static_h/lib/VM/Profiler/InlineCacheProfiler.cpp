/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMESVM_PROFILER_BB
#ifndef INLINECACHE_PROFILER_H
#include "hermes/VM/Profiler/InlineCacheProfiler.h"
#include "hermes/Support/Algorithms.h"
#include "hermes/VM/CodeBlock.h"
#include "hermes/VM/Handle.h"
#include "hermes/VM/HiddenClass.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StringView.h"
#include "hermes/VM/SymbolID.h"

#include <iomanip>
#include <sstream>
#include <vector>

namespace hermes {
namespace vm {

InlineCacheProfiler::ICMiss &InlineCacheProfiler::getICMissBySourceLocation(
    CodeBlock *codeblock,
    uint32_t instOffset) {
  // if not exist, create inline caching entry for the source location
  ICSrcKey icKey(instOffset, codeblock);
  auto it = cacheMisses_.find(icKey);
  if (it == cacheMisses_.end()) {
    ICMiss icMiss;
    auto p = cacheMisses_.try_emplace(icKey, icMiss);
    assert(p.second && "Guaranteed to insert");
    it = p.first;
  }
  return it->second;
}

bool InlineCacheProfiler::insertICMiss(
    CodeBlock *codeblock,
    uint32_t instOffset,
    SymbolID &propertyID,
    ClassId objectHiddenClassId,
    ClassId cachedHiddenClassId) {
  ICMiss &icMiss = getICMissBySourceLocation(codeblock, instOffset);
  // record the hidden class pair for the source location
  auto hcPair =
      std::pair<ClassId, ClassId>(objectHiddenClassId, cachedHiddenClassId);
  auto icRecord =
      std::pair<PropertyId, HiddenClassPair>(propertyID.unsafeGetRaw(), hcPair);
  icMiss.insertMiss(icRecord);

  ++totalMisses_;
  return true;
}

bool InlineCacheProfiler::insertICHit(
    CodeBlock *codeblock,
    uint32_t instOffset) {
  // if not exist, create inline caching entry for the source location
  ICMiss &icMiss = getICMissBySourceLocation(codeblock, instOffset);
  icMiss.incrementHit();

  ++totalHits_;
  return true;
}

JSArray *&InlineCacheProfiler::getHiddenClassArray() {
  return cachedHiddenClassesRawPtr_;
}

void InlineCacheProfiler::setHiddenClassArray(JSArray *hiddenClassArray) {
  cachedHiddenClassesRawPtr_ = hiddenClassArray;
}

uint32_t &InlineCacheProfiler::getHiddenClassArrayIndex() {
  return hcIdx_;
}

llvh::DenseMap<InlineCacheProfiler::ClassId, int32_t>
    &InlineCacheProfiler::getClassIdtoIndexMap() {
  return classIdToIdx_;
}

void InlineCacheProfiler::dumpHiddenClassProperties(
    llvh::raw_ostream &ostream,
    HiddenClass *hc,
    Runtime &runtime) {
  ostream << "\t\t";
  if (hc == nullptr) {
    ostream << "[none]\n";
    return;
  }
  if (hc->isDictionary()) {
    ostream << "[dict] ";
  }
  ostream << "<";
  bool first = true;
  HiddenClass::forEachPropertyNoAlloc(
      hc,
      runtime,
      [&ostream, &runtime, &first](
          SymbolID symId, NamedPropertyDescriptor desc) {
        if (first) {
          first = false;
        } else {
          ostream << ", ";
        }
        ostream << runtime.convertSymbolToUTF8(symId);
      });
  ostream << ">\n";
}

std::unique_ptr<InlineCacheProfiler::ICMissList>
InlineCacheProfiler::getRankedInlineCachingMisses() {
  std::unique_ptr<InlineCacheProfiler::ICMissList> icInfoList =
      std::make_unique<InlineCacheProfiler::ICMissList>();
  // rank inline caching miss information
  for (auto itr = cacheMisses_.begin(); itr != cacheMisses_.end(); ++itr) {
    icInfoList->push_back(*itr);
  }
  auto compare = [](std::pair<ICSrcKey, ICMiss> &a,
                    std::pair<ICSrcKey, ICMiss> &b) {
    // sort in descending order
    return a.second.missCount > b.second.missCount;
  };
  std::sort(icInfoList->begin(), icInfoList->end(), compare);
  return icInfoList;
}

void InlineCacheProfiler::dumpInlineCachingMissRecord(
    ICMissKey &icInfo,
    uint64_t icMiss,
    Runtime &runtime,
    llvh::raw_ostream &ostream) {
  GCScope gcscope(runtime);

  // dump get property name and frequency
  auto propSymbolID = SymbolID::unsafeCreate(icInfo.first);
  auto propName = runtime.convertSymbolToUTF8(propSymbolID);
  ostream << "\tproperty: " << propName << ", inline cache misses: " << icMiss
          << "\n";

  // dump the information of hidden class layout to ostream
  HiddenClassPair hcPair = icInfo.second;
  HiddenClass *objectHiddenClass = runtime.resolveHiddenClassId(hcPair.first);
  HiddenClass *cachedHiddenClass = runtime.resolveHiddenClassId(hcPair.second);
  dumpHiddenClassProperties(ostream, objectHiddenClass, runtime);
  dumpHiddenClassProperties(ostream, cachedHiddenClass, runtime);
  ostream << "\n";
  ostream.flush();
}

void InlineCacheProfiler::dumpInfoOfSourceLocation(
    ICSrcKey &srcLoc,
    ICMiss &icMiss,
    Runtime &runtime,
    llvh::raw_ostream &ostream) {
  // get filename, line number, and column number
  auto locOrNull = runtime.getIPSourceLocation(
      srcLoc.second, srcLoc.second->getOffsetPtr(srcLoc.first));

  // output information at the source location
  if (locOrNull.hasValue()) {
    // output the source location info
    auto loc = locOrNull.getValue();
    ostream << "[" << std::get<0>(loc) << ":" << std::get<1>(loc) << ":"
            << std::get<2>(loc) << "] ";

    // output inline caching statistics
    std::stringstream stream;
    stream << std::fixed << std::setprecision(1)
           << (1. * icMiss.missCount) / (icMiss.missCount + icMiss.hitCount);
    std::string missRatio = stream.str();
    ostream << "total access: " << icMiss.missCount + icMiss.hitCount
            << ", miss ratio: " << missRatio << "\n";
  } else {
    ostream << "[No Loc]\n";
  }
}

/// Dumps most frequent inline caching miss source locations and their
/// detailed information, which include inline caching statistics and
/// hidden class layouts at the source location.
/// The source locations are ranked in the descending order of IC misses.
///
/// An example of output for a specific source location is as follows:
/// [filename:line:column] total access: 2661, miss ratio: 0.3
///  property: children, inline cache misses: 427
///    <type, domNamespace, children, childIndex, context, footer>
///    <domNamespace, type, children, childIndex, context, footer>
///
///  property: children, inline cache misses: 427
///    <domNamespace, type, children, childIndex, context, footer>
///    <type, domNamespace, children, childIndex, context, footer>
///  ...
void InlineCacheProfiler::dumpRankedInlineCachingMisses(
    Runtime &runtime,
    llvh::raw_ostream &ostream) {
  // rank the inline caching misses
  std::shared_ptr<InlineCacheProfiler::ICMissList> icInfoList =
      getRankedInlineCachingMisses();

  uint64_t recordPrinted = 0;
  // enumerate each source location where inline caching miss happens
  for (auto &cacheMissEntry : *icInfoList) {
    ICMiss &icMiss = cacheMissEntry.second;
    // print only top 10 location
    if (recordPrinted++ >= 10) {
      break;
    }

    // dump general information at the source location
    ICSrcKey &srcLoc = cacheMissEntry.first;
    dumpInfoOfSourceLocation(srcLoc, icMiss, runtime, ostream);

    // dump details of each type of inline caching misses at the source location
    for (auto &missHCEntry : icMiss.hiddenClasses) {
      ICMissKey &icInfo = missHCEntry.first;
      uint64_t icMiss = missHCEntry.second;
      dumpInlineCachingMissRecord(icInfo, icMiss, runtime, ostream);
    }
  }
}

} // namespace vm
} // namespace hermes
#endif
#endif
