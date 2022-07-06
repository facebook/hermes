/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_STACK_TRACES_TREE_H
#define HERMES_STACK_TRACES_TREE_H

#include "hermes/VM/Runtime.h"
#include "hermes/VM/StackTracesTree-NoRuntime.h"

#include <memory>

namespace hermes {
namespace vm {

#ifdef HERMES_MEMORY_INSTRUMENTATION

/// This is the root of a tree encoding stack-traces for use by the allocation
/// location tracker. The idea is to minimize the amount of storage needed per
/// stack-trace, and time taken to capture a trace by exploiting the fact that
/// many traces will have a common prefix. An alternative but simpler
/// implementation would just capture a full stack-trace as a string per
/// allocated object.
///
/// Each node in the tree represents a bytecode location, and a single
/// stack-trace is represented by leaf nodes in the tree. For a given leaf we
/// can re-construct the call-stack by walking up from the leaf to the root.
///
/// Operation is as follows:
/// * A single StackTracesTree instance lives in Runtime.
/// * Every time a CodeBlock is entered, \c pushCallStack() must be called, when
///   exited \c popCallStack() must be called. These are O(1) operations
///   (relative to current stack size).
/// * To capture a stack trace, call \c getStackTrace(). This is also an O(1) as
///   we have been tracking where we are in the stack up to this point.
/// * Whenever allocation location tracking is first enabled we must call
///   \c syncWithRuntimeStack() which os O(size of current stack) to initially
///   populate the tree.
///
/// Whenever \c pushCallStack() is called a new entry in the tree is only
/// created if there isn't an existing node for the current "code-location".
/// Code location in this case is distinguished by [script name, line no., col.
/// no.] as these are the values which are important when re-constructing a
/// stack-trace. See StackTracesTree-NoRuntime.h for more details.
///
/// All strings used in the trace (function names, script names, etc.) are
/// encoded as indexes into a single table to reduce memory usage. This table
/// can be shared with the heap snapshot infrastructure as Chrome snapshots also
/// use this mapping.
struct StackTracesTree {
  StackTracesTree();

  /// Must be called at the start of tracking the current call stack.
  void syncWithRuntimeStack(Runtime &runtime);

  /// Get the root of the tree.
  StackTracesTreeNode *getRootNode() const;

  /// Must be called before or at every entry to a \c CodeBlock .
  void
  pushCallStack(Runtime &runtime, const CodeBlock *codeBlock, const Inst *ip);
  /// Must pair with every call to \c pushCallStack() .
  void popCallStack();

  /// Return a leaf-node in the tree which can be used to reconstruct a
  /// stack-trace.
  StackTracesTreeNode *
  getStackTrace(Runtime &runtime, const CodeBlock *codeBlock, const Inst *ip);

  /// Get the string table. This is used by heap-snapshot functions as part of
  /// writing out data in Chrome's snapshot format which itself requires a table
  /// of all string data used in the snapshot.
  std::shared_ptr<StringSetVector> getStringTable() const {
    return strings_;
  }

  /// Used in tests to assert the stack is cleared when execution is over
  bool isHeadAtRoot() {
    return head_ == root_.get();
  }

 private:
  StackTracesTreeNode::SourceLoc computeSourceLoc(
      Runtime &runtime,
      const CodeBlock *codeBlock,
      uint32_t bytecodeOffset);

  /// String data is stored in a shared_ptr so it can be shared with code which
  /// writes out heap-snapshots.
  std::shared_ptr<StringSetVector> strings_;

  /// Pre-computed string IDs
  const StringSetVector::size_type rootFunctionID_;
  const StringSetVector::size_type rootScriptNameID_;
  const StringSetVector::size_type nativeFunctionID_;
  const StringSetVector::size_type anonymousFunctionID_;

  /// Every node in the try gets an ID which is used when writing out snapshot
  /// data for Chrome.
  size_t nextNodeID_{1};

  /// The root of the tree is a sentinel which is always present and does not
  /// represent a valid code location.
  std::unique_ptr<StackTracesTreeNode> root_{new StackTracesTreeNode(
      nextNodeID_++,
      nullptr, /* parent */
      {rootScriptNameID_, 0, 0, 0},
      nullptr, /* codeBlock */
      nullptr, /* ip */
      rootFunctionID_)};

  /// Current head of the tree, typically representing the last known call-site
  /// in bytecode execution.
  StackTracesTreeNode *head_;

  /// This vector is the root of all data held in the tree. We hold this here,
  /// rather than allowing the data to avoid an excessive call-stack when
  /// deallocating the tree. Without this, Hermes tests which exercise
  /// JS stack overflow cause C++ stack overflow.
  llvh::SmallVector<std::unique_ptr<StackTracesTreeNode>, 1024> nodes_;
};

#endif // HERMES_MEMORY_INSTRUMENTATION

} // namespace vm
} // namespace hermes
#endif // HERMES_STACK_TRACES_TREE_H
