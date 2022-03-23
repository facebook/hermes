/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_PROFILER_CHROMETRACESERIALIZERPOSIX_H
#define HERMES_VM_PROFILER_CHROMETRACESERIALIZERPOSIX_H

// TODO: Remove dependency on SamplingProfilerPosix from ChromeTraceSerializer.
// A new header may need to be introduced for data entities. It may make sense
// to share the data entity across different SamplingProfiler implementations.

/// This file convert sampled stack frames into Chrome trace format which
/// is documented here:
/// https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/preview

#include "hermes/Support/JSONEmitter.h"
#include "hermes/VM/Profiler/SamplingProfiler.h"

#include "llvh/ADT/DenseMap.h"

#include <thread>

namespace hermes {
namespace vm {

/// Generating next id for stack frame.
class ChromeFrameIdGenerator {
  uint32_t nextFrameId_{1};

 public:
  uint32_t getNextFrameNodeId() {
    return nextFrameId_++;
  }
};

/// Represent a single stack frame node in collapsed/merged call tree
/// in chrome trace format.
class ChromeStackFrameNode {
 private:
  /// Unique id for the stack frame.
  uint32_t id_;
  /// Frame information. Is None iff this is the root node.
  llvh::Optional<SamplingProfiler::StackFrame> frameInfo_;
  /// All callee/children of this stack frame.
  std::vector<std::shared_ptr<ChromeStackFrameNode>> children_;
  /// How many times was this node on the top of the callstack during profiling.
  uint32_t hitCount_ = 0;

  /// \p node represents the current visiting node.
  /// \p parent can be nullptr for root node.
  using DfsWalkCallback = const std::function<void(
      const ChromeStackFrameNode &node,
      const ChromeStackFrameNode *parent)>;

 private:
  void dfsWalkHelper(
      DfsWalkCallback &callback,
      const ChromeStackFrameNode *parent) const {
    callback(*this, parent);
    for (const auto &child : children_) {
      child->dfsWalkHelper(callback, this);
    }
  }

 public:
  explicit ChromeStackFrameNode(
      uint32_t nextFrameId,
      llvh::Optional<SamplingProfiler::StackFrame> frame)
      : id_(nextFrameId), frameInfo_(frame) {}

  uint32_t getId() const {
    return id_;
  }

  // Get the frame info for this node. Should never be called on the root node
  // since it will be None.
  const SamplingProfiler::StackFrame &getFrameInfo() const {
    return *frameInfo_;
  }

  /// Increments this node's hit counter by one.
  void addHit() {
    ++hitCount_;
  }

  /// \return this node's hit counter.
  uint32_t getHitCount() const {
    return hitCount_;
  }

  /// Find a child node matching \p target, otherwise add \p target
  /// as a new child.
  /// \return the found/added child node.
  std::shared_ptr<ChromeStackFrameNode> findOrAddNewChild(
      ChromeFrameIdGenerator &frameIdGen,
      const SamplingProfiler::StackFrame &target);

  /// DFS walk the call tree using current node as root.
  /// For each visited node, invoke \p callback.
  void dfsWalk(DfsWalkCallback &callback) const {
    this->dfsWalkHelper(callback, nullptr);
  }

  /// Get the vector with the this node's children list.
  const std::vector<std::shared_ptr<ChromeStackFrameNode>> &getChildren()
      const {
    return children_;
  }
};

/// Represent an OS sample event(without duration) in chrome format.
class ChromeSampleEvent {
 private:
  // TODO: get real cpu id.
  int cpu_{-1};
  // Seems should always be one.
  int weight_{1};
  SamplingProfiler::ThreadId tid_;
  SamplingProfiler::TimeStampType timeStamp_;
  std::shared_ptr<ChromeStackFrameNode> leafNode_;

 public:
  explicit ChromeSampleEvent(
      SamplingProfiler::ThreadId tid,
      SamplingProfiler::TimeStampType timeStamp,
      std::shared_ptr<ChromeStackFrameNode> leaf)
      : tid_(tid), timeStamp_(timeStamp), leafNode_(leaf) {}

  /// \return CPU id.
  int getCpu() const {
    return cpu_;
  }

  /// \return weight.
  int getWeight() const {
    return weight_;
  }

  /// Thread id of this event.
  SamplingProfiler::ThreadId getTid() const {
    return tid_;
  }

  /// Timestamp when this event occurred.
  SamplingProfiler::TimeStampType getTimeStamp() const {
    return timeStamp_;
  }

  /// \return leaf frame of the stack in call tree corresponding to
  /// this instant sample event.
  std::shared_ptr<ChromeStackFrameNode> getLeafNode() const {
    return leafNode_;
  }
};

/// Represent all data for a trace session in chrome trace format.
class ChromeTraceFormat {
 private:
  /// Id of target process.
  uint32_t pid_;
  /// Thread names map.
  SamplingProfiler::ThreadNamesMap threadNames_;
  /// The root of the stack frame tree.
  const std::shared_ptr<ChromeStackFrameNode> root_;
  /// Maintain all transformed chrome sample events.
  std::vector<ChromeSampleEvent> sampleEvents_;

  explicit ChromeTraceFormat(
      uint32_t pid,
      const SamplingProfiler::ThreadNamesMap &threadNames,
      std::unique_ptr<ChromeStackFrameNode> root)
      : pid_(pid), threadNames_(threadNames), root_(std::move(root)) {}

 public:
  static ChromeTraceFormat create(
      uint32_t pid,
      const SamplingProfiler::ThreadNamesMap &threadNames,
      const std::vector<SamplingProfiler::StackTrace> &sampledStacks);

  uint32_t getPid() const {
    return pid_;
  }

  const SamplingProfiler::ThreadNamesMap &getThreadNames() const {
    return threadNames_;
  }

  const ChromeStackFrameNode &getRoot() const {
    return *root_;
  }

  const std::vector<ChromeSampleEvent> &getSampledEvents() const {
    return sampleEvents_;
  }
};

/// Serialize input ChromeTraceFormat to output stream.
class ChromeTraceSerializer {
 private:
  ChromeTraceFormat trace_;
  SamplingProfiler::TimeStampType firstEventTimeStamp_;

 private:
  // Emit process_name metadata event.
  void serializeProcessName(JSONEmitter &json) const;
  // Emit threads related events.
  void serializeThreads(JSONEmitter &json) const;
  // Emit "sampled" events for captured stack traces.
  void serializeSampledEvents(JSONEmitter &json) const;
  // Emit "stackFrames" entries.
  void serializeStackFrames(JSONEmitter &json) const;

  // \return a serializable timeStamp string.
  static std::string getSerializedTimeStamp(
      SamplingProfiler::TimeStampType timeStamp);

 public:
  explicit ChromeTraceSerializer(ChromeTraceFormat &&chromeTrace);

  /// Serialize chrome trace to \p OS.
  void serialize(llvh::raw_ostream &OS) const;
};

/// Serialize the \p chromeTrace as a Profiler.Profile to \p os. See the url
/// below for a description of that type.
///
/// https://chromedevtools.github.io/devtools-protocol/tot/Profiler/#type-Profile
///
void serializeAsProfilerProfile(
    llvh::raw_ostream &os,
    ChromeTraceFormat &&chromeTrace);

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_PROFILER_CHROMETRACESERIALIZERPOSIX_H
