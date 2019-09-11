/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef INLINECACHE_PROFILER_H
#define INLINECACHE_PROFILER_H

#include "hermes/VM/SymbolID.h"
#include "llvm/ADT/DenseMap.h"

namespace hermes {
namespace vm {

class CodeBlock;
class HiddenClass;
class JSArray;

class InlineCacheProfiler {
 public:
  using ClassId = uint64_t;
  using PropertyId = uint32_t;
  using ICSrcKey = std::pair<uint32_t, CodeBlock *>;
  using HiddenClassPair = std::pair<ClassId, ClassId>;
  using ICMissKey = std::pair<PropertyId, HiddenClassPair>;

  /// Stores all inline caching misses at one specific source location, which
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

    /// Total number of inline caching misses at the source location.
    uint32_t missCount{0};

    /// Internal map that keeps track of the mapping between
    /// <property, object hidden class, cached hidden class> and its frequency.
    llvm::DenseMap<ICMissKey, uint32_t> hiddenClasses;
  };

  /// Add an inline caching miss record.
  bool insertICMiss(
      CodeBlock *codeblock,
      uint32_t instOffset,
      SymbolID &propertyID,
      ClassId objectHiddenClassId,
      ClassId cachedHiddenClassId);

  /// Get the total number of inline caching misses.
  uint32_t getTotalMisses() {
    return totalMisses_;
  }

  /// Get a JS array containing all hidden classes that shouldn't be
  /// garbage collected.
  JSArray *getHiddenClassArray();

  /// Set a JS array containing all hidden classes that shouldn't be
  /// garbage collected.
  void setHiddenClassArray(JSArray *hiddenClassArray);

  /// Get the current index of element in the hidden class array
  /// used by the inline caching profiler.
  uint32_t &getHiddenClassArrayIndex();

  /// Get a map from object Id to array index in the array
  /// referenced by cachedHiddenClassesRawPtr_.
  llvm::DenseMap<InlineCacheProfiler::ClassId, int32_t> &getClassIdtoIndexMap();

 private:
  /// Keep track of the current index of the hidden class array
  /// used by the inline caching profiler.
  uint32_t hcIdx_{0};

  /// Store an array of hidden classes that will be used
  /// by inline caching profiler.
  JSArray *cachedHiddenClassesRawPtr_;

  /// Store a map from object Id to array index in the array
  /// referenced by cachedHiddenClassesRawPtr_.
  llvm::DenseMap<InlineCacheProfiler::ClassId, int32_t> classIdToIdx_;

  /// Total number of inline caching misses during the program execution.
  uint32_t totalMisses_{0};

  /// Store the data structure of all inline caching misses information.
  /// The map is keyed by pairs <instruction offset, CodeBlock> and maps
  /// to ICMiss objects, which keeps track of hidden classes and frequency.
  llvm::DenseMap<ICSrcKey, ICMiss> cacheMisses_;
};

} // namespace vm
} // namespace hermes

#endif
