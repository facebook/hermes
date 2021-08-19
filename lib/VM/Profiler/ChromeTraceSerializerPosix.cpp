/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#if !defined(_WINDOWS) && !defined(__EMSCRIPTEN__)
#include "hermes/VM/Profiler/ChromeTraceSerializerPosix.h"

#include "hermes/VM/JSNativeFunctions.h"

namespace hermes {
namespace vm {

static std::shared_ptr<ChromeStackFrameNode> findOrAddNewHelper(
    ChromeFrameIdGenerator &frameIdGen,
    std::vector<std::shared_ptr<ChromeStackFrameNode>> &v,
    const SamplingProfiler::StackFrame &target) {
  auto resIter = std::find_if(
      v.begin(),
      v.end(),
      [&target](const std::shared_ptr<ChromeStackFrameNode> &node) {
        return node->getFrameInfo() == target;
      });
  if (resIter != v.end()) {
    return *resIter;
  }
  v.emplace_back(std::make_shared<ChromeStackFrameNode>(
      frameIdGen.getNextFrameNodeId(), target));
  assert(v.size() > 0);
  return v.back();
}

std::shared_ptr<ChromeStackFrameNode> ChromeStackFrameNode::findOrAddNewChild(
    ChromeFrameIdGenerator &frameIdGen,
    const SamplingProfiler::StackFrame &target) {
  return findOrAddNewHelper(frameIdGen, children_, target);
}

/*static*/ ChromeTraceFormat ChromeTraceFormat::create(
    uint32_t pid,
    const SamplingProfiler::ThreadNamesMap &threadNames,
    const std::vector<SamplingProfiler::StackTrace> &sampledStacks) {
  ChromeFrameIdGenerator frameIdGen;
  ChromeTraceFormat trace{pid, threadNames};
  for (const SamplingProfiler::StackTrace &sample : sampledStacks) {
    std::shared_ptr<ChromeStackFrameNode> leafNode;
    assert(sample.stack.size() > 0 && "Why does the sample have no stack?");

    bool isRootFrame = true;
    // Leaf frame is in sample[0] so dump it backward to
    // get root => leaf represenation.
    for (auto iter = sample.stack.rbegin(); iter != sample.stack.rend();
         ++iter) {
      const SamplingProfiler::StackFrame &frame = *iter;
      if (isRootFrame) {
        leafNode = findOrAddNewHelper(frameIdGen, trace.callTrees_, frame);
        isRootFrame = false;
      } else {
        leafNode = leafNode->findOrAddNewChild(frameIdGen, frame);
      }
    }
    assert(leafNode != nullptr && "Why can't we find a leaf node?");
    trace.sampleEvents_.emplace_back(sample.tid, sample.timeStamp, leafNode);
  }
  return trace;
}

ChromeTraceSerializer::ChromeTraceSerializer(ChromeTraceFormat &&chromeTrace)
    : trace_(std::move(chromeTrace)) {
  firstEventTimeStamp_ = trace_.getSampledEvents().empty()
      ? std::chrono::steady_clock::now()
      : trace_.getSampledEvents()[0].getTimeStamp();
}

enum class ChromeEventType {
  Completed = 0,
  Instant = 1,
  Metadata = 2,
};
static const char *ChromeEventTypeNames[] = {"X", "i", "M"};

void ChromeTraceSerializer::serializeProcessName(JSONEmitter &json) const {
  double pid = trace_.getPid();
  json.openDict();
  {
    json.emitKeyValue("name", "process_name");
    json.emitKeyValue(
        "ph",
        ChromeEventTypeNames[static_cast<uint32_t>(ChromeEventType::Metadata)]);
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
      json.emitKeyValue(
          "ph",
          ChromeEventTypeNames[static_cast<uint32_t>(
              ChromeEventType::Metadata)]);
      json.emitKeyValue("cat", "__metadata");
      json.emitKeyValue("pid", static_cast<double>(pid));
      // Use first event time for thread_name time.
      json.emitKeyValue("ts", getSerializedTimeStamp(firstEventTimeStamp_));
      json.emitKeyValue("tid", oscompat::to_string(tid));

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
      json.emitKeyValue(
          "ph",
          ChromeEventTypeNames[static_cast<uint32_t>(
              ChromeEventType::Completed)]);
      json.emitKeyValue("dur", 0.0);
      json.emitKeyValue("pid", static_cast<double>(pid));
      json.emitKeyValue("ts", getSerializedTimeStamp(firstEventTimeStamp_));
      json.emitKeyValue("tid", oscompat::to_string(tid));

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
    json.emitKeyValue("cpu", oscompat::to_string(sample.getCpu()));
    json.emitKeyValue("name", "");
    json.emitKeyValue("ts", getSerializedTimeStamp(sample.getTimeStamp()));
    json.emitKeyValue("pid", static_cast<double>(pid));
    json.emitKeyValue("tid", oscompat::to_string(sample.getTid()));
    json.emitKeyValue("weight", oscompat::to_string(sample.getWeight()));

    double stackId = sample.getLeafNode()->getId();
    assert(stackId > 0 && "Invalid stack id");
    json.emitKeyValue("sf", stackId);
    json.closeDict();
  }
}

void ChromeTraceSerializer::serializeStackFrames(JSONEmitter &json) const {
  for (const auto &tree : trace_.getCallTree()) {
    tree->dfsWalk([&json](
                      const ChromeStackFrameNode &node,
                      const ChromeStackFrameNode *parent) {
      json.emitKey(oscompat::to_string(node.getId()));
      json.openDict();

      auto getJSFunctionName = [](hbc::BCProvider *bcProvider,
                                  uint32_t funcId) {
        hbc::RuntimeFunctionHeader functionHeader =
            bcProvider->getFunctionHeader(funcId);
        return bcProvider->getStringRefFromID(functionHeader.functionName())
            .str();
      };

      auto getSourceLocation =
          [](hbc::BCProvider *bcProvider,
             uint32_t funcId,
             uint32_t opcodeOffset) -> OptValue<hbc::DebugSourceLocation> {
        const hbc::DebugOffsets *debugOffsets =
            bcProvider->getDebugOffsets(funcId);
        if (debugOffsets != nullptr &&
            debugOffsets->sourceLocations != hbc::DebugOffsets::NO_OFFSET) {
          return bcProvider->getDebugInfo()->getLocationForAddress(
              debugOffsets->sourceLocations, opcodeOffset);
        }
        return llvh::None;
      };

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
            std::string fileNameStr =
                bcProvider->getDebugInfo()->getFilenameByID(
                    sourceLocOpt.getValue().filenameId);

            uint32_t line = sourceLocOpt.getValue().line;
            uint32_t column = sourceLocOpt.getValue().column;
            // format: frame_name(file:line:column)
            os << "(" << fileNameStr << ":" << line << ":" << column << ")";
            // Still emit line/column entries for babel/metro/prepack
            // source map symbolication.
            json.emitKeyValue("line", oscompat::to_string(line));
            json.emitKeyValue("column", oscompat::to_string(column));

            // Emit function's start line/column so that can we symbolicate
            // name correctly.
            OptValue<hbc::DebugSourceLocation> funcStartSourceLocOpt =
                getSourceLocation(bcProvider, frame.jsFrame.functionId, 0);
            if (funcStartSourceLocOpt.hasValue()) {
              json.emitKeyValue(
                  "funcLine",
                  oscompat::to_string(funcStartSourceLocOpt.getValue().line));
              json.emitKeyValue(
                  "funcColumn",
                  oscompat::to_string(funcStartSourceLocOpt.getValue().column));
            }
          } else {
            // Without debug info, emit virtual address for source map
            // symbolication.
            uint32_t funcVirtAddr = bcProvider->getVirtualOffsetForFunction(
                frame.jsFrame.functionId);
            json.emitKeyValue(
                "funcVirtAddr", oscompat::to_string(funcVirtAddr));
            json.emitKeyValue(
                "offset", oscompat::to_string(frame.jsFrame.offset));
          }
          break;
        }

        case SamplingProfiler::StackFrame::FrameKind::NativeFunction: {
          frameName =
              std::string("[Native] ") + getFunctionName(frame.nativeFrame);
          categoryName = "Native";
          break;
        }

        case SamplingProfiler::StackFrame::FrameKind::
            FinalizableNativeFunction: {
          // TODO: find a way to get host function name out of
          // FinalizableNativeFunction.
          frameName = "[HostFunction]";
          categoryName = "Native";
          break;
        }

        case SamplingProfiler::StackFrame::FrameKind::GCFrame: {
          if (frame.gcFrame != nullptr) {
            frameName = std::string("[GC ") + *frame.gcFrame + "]";
          } else {
            frameName = "[GC]";
          }
          categoryName = "Metadata";
          break;
        }

        default:
          llvm_unreachable("Unknown frame kind");
      }

      json.emitKeyValue("name", frameName);
      json.emitKeyValue("category", categoryName);
      // Root node does not have "parent" field.
      if (parent != nullptr) {
        json.emitKeyValue("parent", static_cast<double>(parent->getId()));
      }
      json.closeDict();
    });
  }
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
  return oscompat::to_string(
      std::chrono::duration_cast<std::chrono::microseconds>(
          timeStamp.time_since_epoch())
          .count());
}

} // namespace vm
} // namespace hermes

#endif // not _WINDOWS
