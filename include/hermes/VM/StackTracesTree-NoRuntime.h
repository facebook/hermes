/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/// \file This header file defines features of StackTracesTree/allocation
/// location tracking without adding a dependency on other VM features. This is
/// primarily needed within the GC implementation. Concrete implementation for
/// features in this file appear in StackTracesTree.cpp.

#ifndef HERMES_STACK_TRACES_TREE_NO_RUNTIME_H
#define HERMES_STACK_TRACES_TREE_NO_RUNTIME_H

#if defined(HERMES_ENABLE_DEBUGGER)
/// \def HERMES_ENABLE_ALLOCATION_LOCATION_TRACES
/// \brief This macro is used to decide if various pieces of code for tracking
/// stack-traces for allocations are enabled. These should be removed from
/// production VM builds as they add significant overhead.
#define HERMES_ENABLE_ALLOCATION_LOCATION_TRACES
#endif

#include <cstdint>
#include <memory>

#include "hermes/Support/StringSetVector.h"
#include "llvm/ADT/DenseMap.h"

namespace hermes {
namespace vm {

struct StackTracesTree;
class CodeBlock;

struct StackTracesTreeNode {
  /// Each node represents a code location defined by this class. Note this is
  /// "less-specific" than IP + CodeBlock, but all that's needed to reconstruct
  /// a call-stack. As such, we have some utility functions here to de-dupe
  /// nodes that would appear at the same "location".
  struct SourceLoc {
    StringSetVector::size_type scriptName;
    int32_t lineNo;
    int32_t columnNo;
    SourceLoc(
        StringSetVector::size_type scriptName,
        int32_t lineNo,
        int32_t columnNo)
        : scriptName(scriptName), lineNo(lineNo), columnNo(columnNo){};

    unsigned hash() const {
      return scriptName ^ columnNo ^ lineNo;
    };

    bool operator==(const SourceLoc &r) const {
      return scriptName == r.scriptName && lineNo == r.lineNo &&
          columnNo == r.columnNo;
    }
  };

  /// Utility class for use with \c llvm::DenseMap .
  struct SourceLocMapInfo {
    static inline SourceLoc getEmptyKey() {
      return {SIZE_MAX, -1, -1};
    }
    static inline SourceLoc getTombstoneKey() {
      return {SIZE_MAX - 1, -1, -1};
    }
    static unsigned getHashValue(const SourceLoc &v) {
      return v.hash();
    }
    static bool isEqual(const SourceLoc &l, const SourceLoc &r) {
      return l == r;
    }
  };

  using ChildBytecodeMap = llvm::DenseMap<uint32_t, StackTracesTreeNode *>;

  // This is supposed to map a CodeBlock* to ChildBytecodeMap, but DenseMap
  // tries to do alignof() on CodeBlock* which isn't allowed on an incomplete
  // type. So I've worked around it by just using void* and casting as needed.
  using ChildCodeblockMap = llvm::DenseMap<void *, ChildBytecodeMap>;

  using ChildSourceLocMap =
      llvm::DenseMap<SourceLoc, StackTracesTreeNode *, SourceLocMapInfo>;

  /// Utility class for iterating over children of this node.
  struct ChildIterator
      : public std::iterator<std::input_iterator_tag, StackTracesTreeNode> {
    ChildIterator(ChildSourceLocMap::iterator sourceLocIt)
        : sourceLocIt_(sourceLocIt) {}

    ChildIterator &operator++() {
      sourceLocIt_++;
      return *this;
    }
    ChildIterator operator++(int) {
      auto retval = *this;
      ++(*this);
      return retval;
    }
    bool operator==(ChildIterator other) const {
      return sourceLocIt_ == other.sourceLocIt_;
    }
    bool operator!=(ChildIterator other) const {
      return !(*this == other);
    }
    StackTracesTreeNode *operator*() const {
      return sourceLocIt_->second;
    }

   private:
    ChildSourceLocMap::iterator sourceLocIt_;
  };

  StackTracesTreeNode(
      size_t id,
      StackTracesTreeNode *parent,
      SourceLoc sourceLoc,
      const CodeBlock *codeBlock,
      const void *ip,
      StringSetVector::size_type name)
      : id(id),
        parent(parent),
        sourceLoc(sourceLoc),
        name(name),
        codeBlock_(codeBlock),
        ip_(ip){};

  // Public data fields for the node.
  const size_t id;
  StackTracesTreeNode *const parent;
  const SourceLoc sourceLoc;
  const StringSetVector::size_type name;

  ChildIterator begin() {
    return {sourceLocToChildMap_.begin()};
  }

  ChildIterator end() {
    return {sourceLocToChildMap_.end()};
  }

 private:
  friend StackTracesTree;

  StackTracesTreeNode *findChild(
      const CodeBlock *codeBlock,
      uint32_t bytecodeOffset) const;

  StackTracesTreeNode *findChild(const SourceLoc &sourceLoc) const;

  void addChild(
      StackTracesTreeNode *child,
      const CodeBlock *codeBlock,
      uint32_t bytecodeOffset,
      SourceLoc sourceLoc);

  void addMapping(
      const CodeBlock *codeBlock,
      uint32_t bytecodeOffset,
      StackTracesTreeNode *node);

  // These fields are used only to stop us adding the same stack frame multiple
  // times. This can happen, for example, with bound-functions and in some other
  // cases. It may be possible to do something smarter in the interpreter loop
  // to avoid the need for these.
  const CodeBlock *codeBlock_;
  /// Can't use Inst directly here, but doesn't matter as all we need is to
  /// compare pointer values.
  const void *ip_;
  /// Keep track of how many calls at this point in the stack sharing the
  /// codeBlock + IP. This value is only used at run-time, allowing us to skip
  /// the correct number of pops before we actually move the tree head above
  /// this stack location. It does not contribute to stack reconstruction.
  int duplicatePushDepth_{0};

  /// Maps SourceLoc to children.
  ChildSourceLocMap sourceLocToChildMap_;

  /// Cache of CodeBlock + IP to child to fast-path when the exact same bytecode
  /// location is used multiple times.
  ChildCodeblockMap codeBlockToChildMap_;
};

} // namespace vm
} // namespace hermes
#endif // HERMES_STACK_TRACES_TREE_NO_RUNTIME_H
