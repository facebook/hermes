/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#if !defined(_WINDOWS) && !defined(__EMSCRIPTEN__)
#include "hermes/VM/Profiler/ChromeTraceSerializerPosix.h"

#include "hermes/VM/JSNativeFunctions.h"

#include <unordered_map>

namespace hermes {
namespace vm {

std::shared_ptr<ChromeStackFrameNode> ChromeStackFrameNode::findOrAddNewChild(
    ChromeFrameIdGenerator &frameIdGen,
    const SamplingProfiler::StackFrame &target) {
  for (const auto &node : children_)
    if (node->getFrameInfo() == target)
      return node;
  children_.emplace_back(std::make_unique<ChromeStackFrameNode>(
      frameIdGen.getNextFrameNodeId(), target));
  return children_.back();
}

/*static*/ ChromeTraceFormat ChromeTraceFormat::create(
    uint32_t pid,
    const SamplingProfiler::ThreadNamesMap &threadNames,
    const std::vector<SamplingProfiler::StackTrace> &sampledStacks) {
  ChromeFrameIdGenerator frameIdGen;
  ChromeTraceFormat trace{
      pid,
      threadNames,
      std::make_unique<ChromeStackFrameNode>(
          frameIdGen.getNextFrameNodeId(), llvh::None)};
  for (const SamplingProfiler::StackTrace &sample : sampledStacks) {
    std::shared_ptr<ChromeStackFrameNode> leafNode = trace.root_;
    // Leaf frame is in sample[0] so dump it backward to
    // get root => leaf represenation.
    for (const auto &frame : llvh::reverse(sample.stack))
      leafNode = leafNode->findOrAddNewChild(frameIdGen, frame);
    leafNode->addHit();
    trace.sampleEvents_.emplace_back(sample.tid, sample.timeStamp, leafNode);
  }
  return trace;
}

namespace {
std::string getJSFunctionName(hbc::BCProvider *bcProvider, uint32_t funcId) {
  hbc::RuntimeFunctionHeader functionHeader =
      bcProvider->getFunctionHeader(funcId);
  return bcProvider->getStringRefFromID(functionHeader.functionName()).str();
}

OptValue<hbc::DebugSourceLocation> getSourceLocation(
    hbc::BCProvider *bcProvider,
    uint32_t funcId,
    uint32_t opcodeOffset) {
  const hbc::DebugOffsets *debugOffsets = bcProvider->getDebugOffsets(funcId);
  if (debugOffsets &&
      debugOffsets->sourceLocations != hbc::DebugOffsets::NO_OFFSET) {
    return bcProvider->getDebugInfo()->getLocationForAddress(
        debugOffsets->sourceLocations, opcodeOffset);
  }
  return llvh::None;
}
} // namespace

ChromeTraceSerializer::ChromeTraceSerializer(ChromeTraceFormat &&chromeTrace)
    : trace_(std::move(chromeTrace)) {
  firstEventTimeStamp_ = trace_.getSampledEvents().empty()
      ? std::chrono::steady_clock::now()
      : trace_.getSampledEvents()[0].getTimeStamp();
}

namespace chrome_event_type {
static const char *Completed = "X";
static const char *Metadata = "M";
} // namespace chrome_event_type

void ChromeTraceSerializer::serializeProcessName(JSONEmitter &json) const {
  double pid = trace_.getPid();
  json.openDict();
  {
    json.emitKeyValue("name", "process_name");
    json.emitKeyValue("ph", chrome_event_type::Metadata);
    json.emitKeyValue("cat", "__metadata");
    json.emitKeyValue("pid", pid);
    // Use first event time for process_name time.
    json.emitKeyValue("ts", getSerializedTimeStamp(firstEventTimeStamp_));
    // process_name event has no real tid.
    json.emitKeyValue("tid", "-1");

    // TODO: get the real process name.
    json.emitKey("args");
    json.openDict();
    json.emitKeyValue("name", "hermes");
    json.closeDict(); // args
  }
  json.closeDict(); // process_name entry.
}

void ChromeTraceSerializer::serializeThreads(JSONEmitter &json) const {
  uint32_t pid = trace_.getPid();
  for (const auto &threadNameEntry : trace_.getThreadNames()) {
    SamplingProfiler::ThreadId tid = threadNameEntry.first;
    std::string threadName = threadNameEntry.second;

    // Emit thread_name entry.
    json.openDict();
    {
      json.emitKeyValue("name", "thread_name");
      json.emitKeyValue("ph", chrome_event_type::Metadata);
      json.emitKeyValue("cat", "__metadata");
      json.emitKeyValue("pid", static_cast<double>(pid));
      // Use first event time for thread_name time.
      json.emitKeyValue("ts", getSerializedTimeStamp(firstEventTimeStamp_));
      json.emitKeyValue("tid", std::to_string(tid));

      json.emitKey("args");
      json.openDict();
      json.emitKeyValue("name", threadName);
      json.closeDict(); // args
    }
    json.closeDict(); // thread_name entry.

    // Emit thread entry.
    json.openDict();
    {
      json.emitKeyValue("name", threadName);
      json.emitKeyValue("cat", threadName);
      json.emitKeyValue("ph", chrome_event_type::Completed);
      json.emitKeyValue("dur", 0.0);
      json.emitKeyValue("pid", static_cast<double>(pid));
      json.emitKeyValue("ts", getSerializedTimeStamp(firstEventTimeStamp_));
      json.emitKeyValue("tid", std::to_string(tid));

      json.emitKey("args");
      json.openDict();
      json.closeDict(); // args
    }
    json.closeDict(); // thread entry.
  }
}

void ChromeTraceSerializer::serializeSampledEvents(JSONEmitter &json) const {
  uint32_t pid = trace_.getPid();
  const auto &sampledEvents = trace_.getSampledEvents();
  for (const ChromeSampleEvent &sample : sampledEvents) {
    json.openDict();
    json.emitKeyValue("cpu", std::to_string(sample.getCpu()));
    json.emitKeyValue("name", "");
    json.emitKeyValue("ts", getSerializedTimeStamp(sample.getTimeStamp()));
    json.emitKeyValue("pid", static_cast<double>(pid));
    json.emitKeyValue("tid", std::to_string(sample.getTid()));
    json.emitKeyValue("weight", std::to_string(sample.getWeight()));

    double stackId = sample.getLeafNode()->getId();
    assert(stackId > 0 && "Invalid stack id");
    json.emitKeyValue("sf", stackId);
    json.closeDict();
  }
}

void ChromeTraceSerializer::serializeStackFrames(JSONEmitter &json) const {
  trace_.getRoot().dfsWalk([&json](
                               const ChromeStackFrameNode &node,
                               const ChromeStackFrameNode *parent) {
    json.emitKey(std::to_string(node.getId()));

    if (!parent) {
      json.openDict();
      json.emitKeyValue("name", "[root]");
      json.emitKeyValue("category", "root");
      json.closeDict();
      return;
    }

    json.openDict();

    std::string frameName, categoryName;
    const auto &frame = node.getFrameInfo();
    switch (frame.kind) {
      case SamplingProfiler::StackFrame::FrameKind::JSFunction: {
        RuntimeModule *module = frame.jsFrame.module;
        hbc::BCProvider *bcProvider = module->getBytecode();

        llvh::raw_string_ostream os(frameName);
        os << getJSFunctionName(bcProvider, frame.jsFrame.functionId);
        categoryName = "JavaScript";

        OptValue<hbc::DebugSourceLocation> sourceLocOpt = getSourceLocation(
            bcProvider, frame.jsFrame.functionId, frame.jsFrame.offset);
        if (sourceLocOpt.hasValue()) {
          // Bundle has debug info.
          std::string fileNameStr = bcProvider->getDebugInfo()->getFilenameByID(
              sourceLocOpt.getValue().filenameId);

          uint32_t line = sourceLocOpt.getValue().line;
          uint32_t column = sourceLocOpt.getValue().column;
          // format: frame_name(file:line:column)
          os << "(" << fileNameStr << ":" << line << ":" << column << ")";
          // Still emit line/column entries for babel/metro/prepack
          // source map symbolication.
          json.emitKeyValue("line", std::to_string(line));
          json.emitKeyValue("column", std::to_string(column));

          // Emit function's start line/column so that can we symbolicate
          // name correctly.
          OptValue<hbc::DebugSourceLocation> funcStartSourceLocOpt =
              getSourceLocation(bcProvider, frame.jsFrame.functionId, 0);
          if (funcStartSourceLocOpt.hasValue()) {
            json.emitKeyValue(
                "funcLine",
                std::to_string(funcStartSourceLocOpt.getValue().line));
            json.emitKeyValue(
                "funcColumn",
                std::to_string(funcStartSourceLocOpt.getValue().column));
          }
        } else {
          // Without debug info, emit virtual address for source map
          // symbolication.
          uint32_t funcVirtAddr =
              bcProvider->getVirtualOffsetForFunction(frame.jsFrame.functionId);
          json.emitKeyValue("funcVirtAddr", std::to_string(funcVirtAddr));
          json.emitKeyValue("offset", std::to_string(frame.jsFrame.offset));
        }
        break;
      }

      case SamplingProfiler::StackFrame::FrameKind::NativeFunction: {
        frameName =
            std::string("[Native] ") + getFunctionName(frame.nativeFrame);
        categoryName = "Native";
        break;
      }

      case SamplingProfiler::StackFrame::FrameKind::FinalizableNativeFunction: {
        // TODO: find a way to get host function name out of
        // FinalizableNativeFunction.
        frameName = "[HostFunction]";
        categoryName = "Native";
        break;
      }

      case SamplingProfiler::StackFrame::FrameKind::SuspendFrame: {
        assert(frame.suspendFrame && "suspendFrame name should never be null");
        frameName = "[" + *frame.suspendFrame + "]";
        categoryName = "Metadata";
        break;
      }

      default:
        llvm_unreachable("Unknown frame kind");
    }

    json.emitKeyValue("name", frameName);
    json.emitKeyValue("category", categoryName);
    json.emitKeyValue("parent", static_cast<double>(parent->getId()));
    json.closeDict();
  });
}

void ChromeTraceSerializer::serialize(llvh::raw_ostream &OS) const {
  JSONEmitter json(OS);

  // The format of the chrome trace is a bit vague. Here are the essential
  // relationship I reverse engineered out:
  // 1. Tracery only supports OS "samples" events now, not InstantEvent.
  // 2. For each "samples" event, its 'tid' field must match a Completed('X')
  // thread event in "traceEvents" section.(only true for
  // chrome://tracing viewer)
  // 3. To give a name to thread, a 'thread_name' metadata record must be
  // emitted to match its 'tid'.
  // 4. "process_name" metadata record gives name to a process.
  // 5. "sample" event's "sf" field points to a stack frame id "key" in
  // "stackFrames" section.

  json.openDict(); // Open trace.

  // Emit process/threads and its metadata events.
  json.emitKey("traceEvents");
  json.openArray();
  serializeProcessName(json);
  serializeThreads(json);
  json.closeArray(); // traceEvents.

  // Emit "samples" events.
  json.emitKey("samples");
  json.openArray();
  serializeSampledEvents(json);
  json.closeArray(); // samples

  // Emit "stackFrames" entries.
  json.emitKey("stackFrames");
  json.openDict();
  serializeStackFrames(json);
  json.closeDict(); // stackFrames.

  json.closeDict(); // Whole trace.
}

/*static*/ std::string ChromeTraceSerializer::getSerializedTimeStamp(
    SamplingProfiler::TimeStampType timeStamp) {
  return std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(
                            timeStamp.time_since_epoch())
                            .count());
}

namespace {
class ProfilerProfileSerializer {
  ProfilerProfileSerializer(const ProfilerProfileSerializer &) = delete;
  ProfilerProfileSerializer &operator=(const ProfilerProfileSerializer &) =
      delete;
  ProfilerProfileSerializer() = delete;

 public:
  ProfilerProfileSerializer(
      JSONEmitter &emitter,
      ChromeTraceFormat &&chromeTrace)
      : json_(emitter), chromeTrace_(std::move(chromeTrace)) {}

  void serialize() const;

 private:
  void emitNodes() const;
  void emitStartTime() const;
  void emitEndTime() const;
  void emitSamples() const;
  void emitTimeDeltas() const;

  JSONEmitter &json_;
  ChromeTraceFormat chromeTrace_;
};

void ProfilerProfileSerializer::serialize() const {
  json_.openDict();

  if (!chromeTrace_.getSampledEvents().empty()) {
    // samples and timeDeltas are optional, so don't emit if there are no
    // samples.
    emitSamples();
    emitTimeDeltas();
  }

  emitNodes();
  emitStartTime();
  emitEndTime();

  json_.closeDict();
}

static void emitProfileNode(
    JSONEmitter &json,
    const ChromeStackFrameNode &node,
    const std::string &name,
    uint32_t scriptId,
    const std::string &url,
    uint32_t lineNumber,
    uint32_t columnNumber) {
  json.openDict();

  json.emitKeyValue("id", node.getId());

  json.emitKey("callFrame");

  json.openDict();
  json.emitKeyValue("functionName", name);
  json.emitKeyValue("scriptId", scriptId);
  json.emitKeyValue("url", url);
  json.emitKeyValue("lineNumber", lineNumber);
  json.emitKeyValue("columnNumber", columnNumber);

  json.closeDict(); // callFrame

  const auto children = node.getChildren();
  if (!children.empty()) {
    json.emitKey("children");

    json.openArray();
    for (const auto &child : children) {
      json.emitValue(child->getId());
    }
    json.closeArray(); // children
  }

  const uint32_t hitCount = node.getHitCount();
  if (hitCount > 0) {
    json.emitKeyValue("hitCount", hitCount);
  }

  json.closeDict(); // node
}

static void processNode(JSONEmitter &json, const ChromeStackFrameNode &node) {
  std::string name;
  std::string url;
  uint32_t scriptId = 0;
  uint32_t lineNumber = 0;
  uint32_t columnNumber = 0;
  const SamplingProfiler::StackFrame &frame = node.getFrameInfo();
  switch (frame.kind) {
    case SamplingProfiler::StackFrame::FrameKind::JSFunction: {
      RuntimeModule *module = frame.jsFrame.module;
      hbc::BCProvider *bcProvider = module->getBytecode();

      llvh::raw_string_ostream os(name);
      os << getJSFunctionName(bcProvider, frame.jsFrame.functionId);

      url = "unknown";
      OptValue<hbc::DebugSourceLocation> sourceLocOpt = getSourceLocation(
          bcProvider, frame.jsFrame.functionId, frame.jsFrame.offset);
      if (sourceLocOpt.hasValue()) {
        // Bundle has debug info.
        scriptId = sourceLocOpt.getValue().filenameId;
        url = bcProvider->getDebugInfo()->getFilenameByID(scriptId);

        lineNumber = sourceLocOpt.getValue().line;
        columnNumber = sourceLocOpt.getValue().column;
        // format: frame_name(file:line:column)
        os << "(" << url << ":" << lineNumber << ":" << columnNumber << ")";
      }
      break;
    }

    case SamplingProfiler::StackFrame::FrameKind::NativeFunction: {
      name = std::string("[Native] ") + getFunctionName(frame.nativeFrame);
      url = "[native]";
      break;
    }

    case SamplingProfiler::StackFrame::FrameKind::FinalizableNativeFunction: {
      name = "[Host Function]";
      url = "[host]";
      break;
    }

    case SamplingProfiler::StackFrame::FrameKind::SuspendFrame: {
      assert(frame.suspendFrame && "suspendFrame should never be nullptr");
      name = "[" + *frame.suspendFrame + "]";
      url = "[suspended]";
      break;
    }

    default:
      llvm_unreachable("Unknown frame kind");
  }

  emitProfileNode(json, node, name, scriptId, url, lineNumber, columnNumber);
}

void ProfilerProfileSerializer::emitNodes() const {
  json_.emitKey("nodes");
  json_.openArray();

  // Emit the root node first as required by CDP.
  const ChromeStackFrameNode *root = &chromeTrace_.getRoot();
  assert(root && "no profile root");
  emitProfileNode(json_, *root, "[root]", 0, "[root]", 0, 0);

  chromeTrace_.getRoot().dfsWalk([this, root](
                                     const ChromeStackFrameNode &node,
                                     const ChromeStackFrameNode *parent) {
    if (&node == root) {
      assert(!parent && "root node should not have parent");
    } else {
      processNode(json_, node);
    }
  });

  json_.closeArray(); // nodes
}

template <typename T>
static uint64_t toMicros(T t) {
  return std::chrono::duration_cast<std::chrono::microseconds>(t).count();
}

static SamplingProfiler::TimeStampType getFirstTimeStamp(
    const std::vector<ChromeSampleEvent> &events) {
  return events.empty() ? SamplingProfiler::TimeStampType{}
                        : events.front().getTimeStamp();
}

static SamplingProfiler::TimeStampType getLastTimeStamp(
    const std::vector<ChromeSampleEvent> &events) {
  return events.empty() ? SamplingProfiler::TimeStampType{}
                        : events.back().getTimeStamp();
}

void ProfilerProfileSerializer::emitStartTime() const {
  json_.emitKeyValue(
      "startTime",
      toMicros(getFirstTimeStamp(chromeTrace_.getSampledEvents())
                   .time_since_epoch()));
}

void ProfilerProfileSerializer::emitEndTime() const {
  json_.emitKeyValue(
      "endTime",
      toMicros(getLastTimeStamp(chromeTrace_.getSampledEvents())
                   .time_since_epoch()));
}

void ProfilerProfileSerializer::emitSamples() const {
  json_.emitKey("samples");

  json_.openArray();
  const auto &sampledEvents = chromeTrace_.getSampledEvents();
  for (const ChromeSampleEvent &sample : sampledEvents) {
    uint32_t stackId = sample.getLeafNode()->getId();
    assert(stackId > 0 && "Invalid stack id");
    json_.emitValue(stackId);
  }

  json_.closeArray(); // samples
}

void ProfilerProfileSerializer::emitTimeDeltas() const {
  json_.emitKey("timeDeltas");

  json_.openArray();

  const auto &sampledEvents = chromeTrace_.getSampledEvents();
  SamplingProfiler::TimeStampType previousTimeStampMicros =
      getFirstTimeStamp(sampledEvents);
  for (const ChromeSampleEvent &sample : sampledEvents) {
    SamplingProfiler::TimeStampType currentTimeStampMicros =
        sample.getTimeStamp();
    json_.emitValue(toMicros(currentTimeStampMicros - previousTimeStampMicros));
    previousTimeStampMicros = currentTimeStampMicros;
  }

  json_.closeArray(); // samples
}
} // namespace

void serializeAsProfilerProfile(
    llvh::raw_ostream &os,
    ChromeTraceFormat &&chromeTrace) {
  JSONEmitter json(os);

  ProfilerProfileSerializer s(json, std::move(chromeTrace));
  s.serialize();
}
} // namespace vm
} // namespace hermes

#endif // not _WINDOWS
