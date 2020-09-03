/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/StackTracesTree-NoRuntime.h"

#ifdef HERMES_ENABLE_ALLOCATION_LOCATION_TRACES

#include "hermes/VM/Callable.h"
#include "hermes/VM/StackFrame-inline.h"
#include "hermes/VM/StackTracesTree.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/StringView.h"

namespace hermes {
namespace vm {

StackTracesTreeNode *StackTracesTreeNode::findChild(
    const CodeBlock *codeBlock,
    uint32_t bytecodeOffset) const {
  auto matchingCodeBlockChildren = codeBlockToChildMap_.find(codeBlock);
  if (matchingCodeBlockChildren != codeBlockToChildMap_.end()) {
    auto matchingOffset =
        matchingCodeBlockChildren->getSecond().find(bytecodeOffset);
    if (matchingOffset != matchingCodeBlockChildren->getSecond().end()) {
      return children_[matchingOffset->getSecond()];
    }
  }
  return nullptr;
}

OptValue<uint32_t> StackTracesTreeNode::findChildIndex(
    const SourceLoc &sourceLoc) const {
  auto matchingChild = sourceLocToChildMap_.find(sourceLoc);
  if (matchingChild != sourceLocToChildMap_.end()) {
    return matchingChild->getSecond();
  }
  return llvh::None;
}

StackTracesTreeNode *StackTracesTreeNode::findChild(
    const SourceLoc &sourceLoc) const {
  auto optIndex = findChildIndex(sourceLoc);
  if (!optIndex.hasValue())
    return nullptr;
  return children_[*optIndex];
}

void StackTracesTreeNode::addChild(
    StackTracesTreeNode *child,
    const CodeBlock *codeBlock,
    uint32_t bytecodeOffset,
    SourceLoc sourceLoc) {
  uint32_t childIndex = children_.size();
  children_.push_back(child);
  bool inserted =
      sourceLocToChildMap_.try_emplace(sourceLoc, childIndex).second;
  (void)inserted;
  assert(inserted && "Tried to add a node for the same sourceLoc twice.");
  addMapping(codeBlock, bytecodeOffset, childIndex);
}

void StackTracesTreeNode::addMapping(
    const CodeBlock *codeBlock,
    uint32_t bytecodeOffset,
    uint32_t childIndex) {
  auto matchingCodeBlockChildren = codeBlockToChildMap_.find(codeBlock);
  if (matchingCodeBlockChildren == codeBlockToChildMap_.end()) {
    ChildBytecodeMap newBytecodeMapping;
    newBytecodeMapping.try_emplace(bytecodeOffset, childIndex);
    codeBlockToChildMap_.try_emplace(
        (void *)codeBlock, std::move(newBytecodeMapping));
  } else {
    auto &bytecodeMapping = matchingCodeBlockChildren->getSecond();
    assert(
        bytecodeMapping.find(bytecodeOffset) == bytecodeMapping.end() &&
        "Tried to add a node for the same codeLoc twice");
    bytecodeMapping.try_emplace(bytecodeOffset, childIndex);
  }
}

StackTracesTree::StackTracesTree()
    : strings_(std::make_shared<StringSetVector>()),
      invalidFunctionID_(strings_->insert("(invalid function name)")),
      invalidScriptNameID_(strings_->insert("(invalid script name)")),
      nativeFunctionID_(strings_->insert("(native)")),
      anonymousFunctionID_(strings_->insert("(anonymous)")),
      head_(root_.get()) {}

void StackTracesTree::syncWithRuntimeStack(Runtime *runtime) {
  head_ = root_.get();

  // Copy the frame pointers into a vector so we can iterate over them in
  // reverse.
  std::vector<StackFramePtr> frames(
      runtime->getStackFrames().begin(), runtime->getStackFrames().end());

  auto frameIt = frames.rbegin();
  if (frameIt == frames.rend()) {
    return;
  }
  // Stack frames tells us the current CodeBlock and the _previous_ IP. So we
  // treat the first frame specially using the IP of the first bytecode in
  // the CodeBlock.
  const CodeBlock *codeBlock = (*frameIt)->getCalleeCodeBlock();
  pushCallStack(runtime, codeBlock, codeBlock->getOffsetPtr(0));
  ++frameIt;
  // The rend() - 1 is to skip the last frame for now as the only way to
  // enable allocationLocationTracker is via a native call triggered from JS. In
  // future we may need to change this depending on how and when tracking is
  // enabled.
  for (; codeBlock && frameIt < frames.rend() - 1; ++frameIt) {
    const Inst *ip = (*frameIt)->getSavedIP();
    pushCallStack(runtime, codeBlock, ip);
    codeBlock = (*frameIt)->getCalleeCodeBlock();
  }
}

StackTracesTreeNode *StackTracesTree::getRootNode() const {
  return root_.get();
}

void StackTracesTree::popCallStack() {
  if (head_->duplicatePushDepth_) {
    head_->duplicatePushDepth_--;
    return;
  }
  head_ = head_->parent;
  assert(head_ && "Pop'ed too far up tree");
}

StackTracesTreeNode::SourceLoc StackTracesTree::computeSourceLoc(
    Runtime *runtime,
    const CodeBlock *codeBlock,
    uint32_t bytecodeOffset) {
  auto location = codeBlock->getSourceLocation(bytecodeOffset);
  // Get filename. If we have a source location, use the filename from
  // that location; otherwise use the RuntimeModule's sourceURL; otherwise
  // report unknown.
  RuntimeModule *runtimeModule = codeBlock->getRuntimeModule();
  std::string scriptName;
  int32_t lineNo, columnNo;
  if (location) {
    scriptName = runtimeModule->getBytecode()->getDebugInfo()->getFilenameByID(
        location->filenameId);
    lineNo = location->line;
    columnNo = location->column;
  } else {
    auto sourceURL = runtimeModule->getSourceURL();
    scriptName = sourceURL.empty() ? "unknown" : sourceURL;
    lineNo = -1;
    columnNo = -1;
  }
  return {strings_->insert(scriptName), lineNo, columnNo};
}

void StackTracesTree::pushCallStack(
    Runtime *runtime,
    const CodeBlock *codeBlock,
    const Inst *ip) {
  assert(codeBlock && ip && "Code block and IP must be known");

  /// This collapses together multiple calls apparently from the same codeBlock
  /// + IP into one node. This can happen with with bound functions, or anything
  /// else where C++ code makes calls into the interpreter without executing
  /// further bytecode. This depth will then be depleted in calls to
  /// \c popCallStack() .
  if (head_->codeBlock_ == codeBlock && head_->ip_ == ip) {
    head_->duplicatePushDepth_++;
    return;
  }

  auto bytecodeOffset = codeBlock->getOffsetOf(ip);
  // Quick-path: Node already exists in tree, and we have a cached mapping for
  // this codeLoc.
  if (auto existingNode = head_->findChild(codeBlock, bytecodeOffset)) {
    head_ = existingNode;
    return;
  }

  // Node exists in tree but doesn't have a pre-computed mapping for this
  // codeBlock + ip. In this case we need to compute the SourceLoc and
  //  a mapping, but can return before we create a new tree node.
  auto sourceLoc = computeSourceLoc(runtime, codeBlock, bytecodeOffset);
  if (OptValue<uint32_t> existingNodeIndex = head_->findChildIndex(sourceLoc)) {
    auto existingNode = head_->children_[*existingNodeIndex];
    assert(existingNode->parent && "Stack trace tree node has no parent");
    existingNode->parent->addMapping(
        codeBlock, bytecodeOffset, *existingNodeIndex);
    head_ = existingNode;
    return;
  }

  // Full-path: Create a new node.

  // TODO: Getting the name in this way works in most cases, but not for things
  // like functions which are dynamically renamed using accessors. E.g.:
  //
  //   function foo() {
  //     return new Object();
  //   }
  //   Object.defineProperty(foo, 'name', {writable:true, value: 'bar'});
  //
  auto nameStr = codeBlock->getNameString(runtime->getHeap().getCallbacks());
  auto nameID =
      nameStr.empty() ? anonymousFunctionID_ : strings_->insert(nameStr);

  auto newNode = hermes::make_unique<StackTracesTreeNode>(
      nextNodeID_++, head_, sourceLoc, codeBlock, ip, nameID);
  auto newNodePtr = newNode.get();
  nodes_.emplace_back(std::move(newNode));
  head_->addChild(newNodePtr, codeBlock, bytecodeOffset, sourceLoc);
  head_ = newNodePtr;
}

StackTracesTreeNode *StackTracesTree::getStackTrace(
    Runtime *runtime,
    const CodeBlock *codeBlock,
    const Inst *ip) {
  if (!codeBlock || !ip) {
    return getRootNode();
  }
  pushCallStack(runtime, codeBlock, ip);
  auto res = head_;
  popCallStack();
  return res;
}

} // namespace vm
} // namespace hermes

#endif // HERMES_ENABLE_ALLOCATION_LOCATION_TRACES
