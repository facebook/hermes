/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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

#include "hermes/Public/DebuggerTypes.h"
#include "hermes/Support/OptValue.h"
#include "hermes/Support/StringSetVector.h"

#include "llvh/ADT/DenseMap.h"

#include <cstdint>
#include <map>
#include <memory>
#pragma GCC diagnostic push

#ifdef HERMES_COMPILER_SUPPORTS_WSHORTEN_64_TO_32
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif
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
    ::facebook::hermes::debugger::ScriptID scriptID;
    int32_t lineNo;
    int32_t columnNo;
    SourceLoc(
        StringSetVector::size_type scriptName,
        ::facebook::hermes::debugger::ScriptID scriptID,
        int32_t lineNo,
        int32_t columnNo)
        : scriptName(scriptName),
          scriptID(scriptID),
          lineNo(lineNo),
          columnNo(columnNo) {}

    unsigned hash() const {
      return scriptName ^ scriptID ^ columnNo ^ lineNo;
    };

    bool operator==(const SourceLoc &r) const {
      return scriptName == r.scriptName && scriptID == r.scriptID &&
          lineNo == r.lineNo && columnNo == r.columnNo;
    }
  };

  /// Utility class for use with \c llvh::DenseMap .
  struct SourceLocMapInfo {
    static inline SourceLoc getEmptyKey() {
      return {SIZE_MAX, 0, -1, -1};
    }
    static inline SourceLoc getTombstoneKey() {
      return {SIZE_MAX - 1, 0, -1, -1};
    }
    static unsigned getHashValue(const SourceLoc &v) {
      return v.hash();
    }
    static bool isEqual(const SourceLoc &l, const SourceLoc &r) {
      return l == r;
    }
  };

  /// Map to index of child in children_.
  using ChildBytecodeMap = llvh::DenseMap<uint32_t, uint32_t>;

  // This is supposed to map a CodeBlock* to ChildBytecodeMap, but DenseMap
  // tries to do alignof() on CodeBlock* which isn't allowed on an incomplete
  // type. So I've worked around it by just using void* and casting as needed.
  using ChildCodeblockMap = llvh::DenseMap<const void *, ChildBytecodeMap>;

  /// Map to index of child in children_.
  using ChildSourceLocMap =
      llvh::DenseMap<SourceLoc, uint32_t, SourceLocMapInfo>;

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

  llvh::ArrayRef<StackTracesTreeNode *> getChildren() const {
    return children_;
  }

 private:
  friend StackTracesTree;

  StackTracesTreeNode *findChild(
      const CodeBlock *codeBlock,
      uint32_t bytecodeOffset) const;

  OptValue<uint32_t> findChildIndex(const SourceLoc &sourceLoc) const;

  StackTracesTreeNode *findChild(const SourceLoc &sourceLoc) const;

  void addChild(
      StackTracesTreeNode *child,
      const CodeBlock *codeBlock,
      uint32_t bytecodeOffset,
      SourceLoc sourceLoc);

  void addMapping(
      const CodeBlock *codeBlock,
      uint32_t bytecodeOffset,
      uint32_t childIndex);

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

  /// List of children registered to this node, indexed from DenseMaps in this
  /// class.
  std::vector<StackTracesTreeNode *> children_{};
};

} // namespace vm
} // namespace hermes
#pragma GCC diagnostic pop

#endif // HERMES_STACK_TRACES_TREE_NO_RUNTIME_H
