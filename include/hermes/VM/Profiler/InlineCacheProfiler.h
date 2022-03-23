/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef INLINECACHE_PROFILER_H
#define INLINECACHE_PROFILER_H

#include "hermes/VM/SymbolID.h"
#include "llvh/ADT/DenseMap.h"

namespace hermes {
namespace inst {
struct Inst;
}

namespace vm {

class CodeBlock;
class HiddenClass;
class JSArray;
class Runtime;

class InlineCacheProfiler {
 public:
  using ClassId = uint64_t;
  using PropertyId = uint32_t;
  using ICSrcKey = std::pair<uint32_t, CodeBlock *>;
  /// A pair of hidden class Ids. The hidden class Id of the object that is
  /// accessed is on the left; and, the hidden class Id of the cached entry
  /// is on the right.
  using HiddenClassPair = std::pair<ClassId, ClassId>;
  /// A key for dentifying inline caching misses.
  /// The key consists of the Id of the property accessed, and HiddenClassPair,
  /// which consists of the hidden class Ids of the object and the cached entry.
  using ICMissKey = std::pair<PropertyId, HiddenClassPair>;

  /// Store all inline caching misses at one specific source location, which
  /// is uniquely identified by code block and instruction offset.
  struct ICMiss {
    /// Increment the inline caching miss count for a pair of hidden classes.
    void insertMiss(ICMissKey icRecord) {
      auto ret =
          hiddenClasses.insert(std::pair<ICMissKey, uint32_t>(icRecord, 1));
      if (!ret.second) {
        ++(ret.first->second);
      }
      ++missCount;
    }

    /// Increment the inline caching hit count for a pair of hidden classes.
    void incrementHit() {
      ++hitCount;
    }

    /// Total number of inline caching misses at the source location.
    uint64_t missCount{0};

    /// Total number of inline caching hits at the source location.
    uint64_t hitCount{0};

    /// Internal map that keeps track of the mapping between
    /// <property, object hidden class, cached hidden class> and its frequency.
    llvh::DenseMap<ICMissKey, uint64_t> hiddenClasses;
  };

  using ICMissList = std::vector<std::pair<ICSrcKey, ICMiss>>;

  /// Get a ICMiss object that contains all information about
  /// inline caching at a specific source location.
  ICMiss &getICMissBySourceLocation(CodeBlock *codeblock, uint32_t instOffset);

  /// Record an inline caching miss.
  bool insertICMiss(
      CodeBlock *codeblock,
      uint32_t instOffset,
      SymbolID &propertyID,
      ClassId objectHiddenClassId,
      ClassId cachedHiddenClassId);

  /// Record an inline caching hit.
  bool insertICHit(CodeBlock *codeblock, uint32_t instOffset);

  /// Get the total number of inline caching misses.
  uint32_t getTotalMisses() {
    return totalMisses_;
  }

  /// Get a JS array containing all hidden classes that shouldn't be
  /// garbage collected.
  JSArray *&getHiddenClassArray();

  /// Set a JS array containing all hidden classes that shouldn't be
  /// garbage collected.
  void setHiddenClassArray(JSArray *hiddenClassArray);

  /// Get the current index of element in the hidden class array
  /// used by the inline caching profiler.
  uint32_t &getHiddenClassArrayIndex();

  /// Get a map from object Id to array index in the array
  /// referenced by cachedHiddenClassesRawPtr_.
  llvh::DenseMap<InlineCacheProfiler::ClassId, int32_t> &getClassIdtoIndexMap();

  /// Organize and rank inline caching misses and dump
  /// the information to ostream. Inline caching miss records are
  /// ranked in descending order based on the miss count.
  void dumpRankedInlineCachingMisses(
      Runtime &runtime,
      llvh::raw_ostream &ostream);

 private:
  /// Keep track of the current index of the hidden class array
  /// used by the inline caching profiler.
  uint32_t hcIdx_{0};

  /// Store an array of hidden classes that will be used
  /// by inline caching profiler.
  JSArray *cachedHiddenClassesRawPtr_{nullptr};

  /// Store a map from object Id to array index in the array
  /// referenced by cachedHiddenClassesRawPtr_.
  llvh::DenseMap<InlineCacheProfiler::ClassId, int32_t> classIdToIdx_;

  /// Rank all inline caching misses records and return a list.
  /// Each source location maintains a inline caching miss record.
  /// All inline caching miss records are ranked in descending order
  /// based on the total miss count at the source locations.
  std::unique_ptr<InlineCacheProfiler::ICMissList>
  getRankedInlineCachingMisses();

  /// Dump source location information and general inline caching statistics
  /// at a specific source location to stream.
  void dumpInfoOfSourceLocation(
      ICSrcKey &srcLoc,
      ICMiss &icMiss,
      Runtime &runtime,
      llvh::raw_ostream &ostream);

  /// Dump a inline caching miss record, which
  /// includes the hidden classes, property, and frequency.
  void dumpInlineCachingMissRecord(
      ICMissKey &icInfo,
      uint64_t icMiss,
      Runtime &runtime,
      llvh::raw_ostream &ostream);

  /// Dump properties of a hidden class to ostream.
  void dumpHiddenClassProperties(
      llvh::raw_ostream &ostream,
      HiddenClass *hc,
      Runtime &runtime);

  /// Total number of inline caching misses during the program execution.
  uint64_t totalMisses_{0};

  /// Total number of inline caching hits during the program execution.
  uint64_t totalHits_{0};

  /// Store the data structure of all inline caching misses information.
  /// The map is keyed by pairs <instruction offset, CodeBlock> and maps
  /// to ICMiss objects, which keeps track of hidden classes and frequency.
  llvh::DenseMap<ICSrcKey, ICMiss> cacheMisses_;
};

} // namespace vm
} // namespace hermes

#endif
