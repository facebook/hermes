/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/HeapSnapshot.h"

#include "hermes/Support/Conversions.h"
#include "hermes/Support/JSONEmitter.h"
#include "hermes/Support/UTF8.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/StackTracesTree.h"
#include "hermes/VM/StringPrimitive.h"

#include <type_traits>

namespace hermes {
namespace vm {

#ifdef HERMES_MEMORY_INSTRUMENTATION

namespace {

/// Lower an instance \p e of enumeration \p Enum to its underlying type.
template <typename Enum>
constexpr typename std::underlying_type<Enum>::type index(Enum e) {
  return static_cast<typename std::underlying_type<Enum>::type>(e);
}

const char *kSectionLabels[] = {
#define V8_SNAPSHOT_SECTION(enumerand, label) label,
#include "hermes/VM/HeapSnapshot.def"
};

} // namespace

HeapSnapshot::HeapSnapshot(JSONEmitter &json, StackTracesTree *stackTracesTree)
    : json_(json),
      stackTracesTree_(stackTracesTree),
      stringTable_(
          stackTracesTree ? stackTracesTree->getStringTable()
                          : std::make_shared<StringSetVector>()) {
  json_.openDict();
  emitMeta();
}

HeapSnapshot::~HeapSnapshot() {
  assert(
      edgeCount_ == expectedEdges_ && "Fewer edges added than were expected");
  emitStrings();
  json_.closeDict(); // top level
}

void HeapSnapshot::beginSection(Section section) {
  auto i = index(nextSection_);

  assert(!sectionOpened_ && "Sections must be explicitly close");
  assert(section != Section::END && "Can't open the end section.");
  assert(
      i <= index(section) &&
      "Trying to open a section after it has already been closed.  Are your "
      "sections ordered correctly?");

  for (; i < index(section); ++i) {
    json_.emitKey(kSectionLabels[i]);
    json_.openArray();
    json_.closeArray();
  }

  json_.emitKey(kSectionLabels[i]);
  json_.openArray();

  nextSection_ = section;
  sectionOpened_ = true;
}

void HeapSnapshot::endSection(Section section) {
  assert(sectionOpened_ && "No section to close");
  assert(section != Section::END && "Can't close the end section.");
  assert(nextSection_ == section && "Closing a different section.");

  json_.closeArray();
  nextSection_ = static_cast<Section>(index(section) + 1);
  sectionOpened_ = false;
}

void HeapSnapshot::beginNode() {
  if (nextSection_ == Section::Edges) {
    // If the edges are being emitted, ignore node output.
    return;
  }
  assert(nextSection_ == Section::Nodes && sectionOpened_);
  // Reset the edge counter.
  currEdgeCount_ = 0;
}

void HeapSnapshot::endNode(
    NodeType type,
    llvh::StringRef name,
    NodeID id,
    HeapSizeType selfSize,
    HeapSizeType traceNodeID) {
  if (nextSection_ == Section::Edges) {
    // If the edges are being emitted, ignore node output.
    return;
  }
  auto &nodeStats = traceNodeStats_[traceNodeID];
  nodeStats.count++;
  nodeStats.size += selfSize;
  assert(nextSection_ == Section::Nodes && sectionOpened_);
  auto res = nodeToIndex_.try_emplace(id, nodeCount_++);
  assert(res.second);
  (void)res;
  json_.emitValue(index(type));
  json_.emitValue(stringTable_->insert(name));
  json_.emitValue(id);
  json_.emitValue(selfSize);
  json_.emitValue(currEdgeCount_);
  json_.emitValue(traceNodeID);
  // detachedness is always zero for hermes, since there's no DOM to attach to.
  json_.emitValue(0);
#ifndef NDEBUG
  expectedEdges_ += currEdgeCount_;
#endif
}

void HeapSnapshot::addNamedEdge(
    EdgeType type,
    llvh::StringRef name,
    NodeID toNode) {
  if (nextSection_ == Section::Nodes) {
    // If we're emitting nodes, only count the number of edges being processed,
    // but don't actually emit them.
    currEdgeCount_++;
    return;
  }
  assert(
      edgeCount_++ < expectedEdges_ && "Added more edges than were expected");
  assert(nextSection_ == Section::Edges && sectionOpened_);

  json_.emitValue(index(type));
  json_.emitValue(stringTable_->insert(name));

  auto nodeIt = nodeToIndex_.find(toNode);
  assert(nodeIt != nodeToIndex_.end());
  // Point to the beginning of the target node in the `nodes` flat array.
  json_.emitValue(nodeIt->second * V8_SNAPSHOT_NODE_FIELD_COUNT);
}

void HeapSnapshot::addIndexedEdge(
    EdgeType type,
    EdgeIndex edgeIndex,
    NodeID toNode) {
  if (nextSection_ == Section::Nodes) {
    // If we're emitting nodes, only count the number of edges being processed,
    // but don't actually emit them.
    currEdgeCount_++;
    return;
  }
  assert(
      edgeCount_++ < expectedEdges_ && "Added more edges than were expected");
  assert(nextSection_ == Section::Edges && sectionOpened_);

  json_.emitValue(index(type));
  json_.emitValue(edgeIndex);

  auto nodeIt = nodeToIndex_.find(toNode);
  assert(nodeIt != nodeToIndex_.end());
  // Point to the beginning of the target node in the `nodes` flat array.
  json_.emitValue(nodeIt->second * V8_SNAPSHOT_NODE_FIELD_COUNT);
}

void HeapSnapshot::addLocation(
    NodeID id,
    ::facebook::hermes::debugger::ScriptID script,
    uint32_t line,
    uint32_t column) {
  assert(
      nextSection_ == Section::Locations && sectionOpened_ &&
      "Shouldn't be emitting locations until the location section starts");
  auto nodeIt = nodeToIndex_.find(id);
  assert(
      nodeIt != nodeToIndex_.end() &&
      "Couldn't add a location for an object that doesn't exist");
  json_.emitValue(nodeIt->second * V8_SNAPSHOT_NODE_FIELD_COUNT);
  json_.emitValue(script);
  // The serialized format uses 0-based indexing for line and column, but the
  // parameters are 1-based.
  assert(line != 0 && "Line should be 1-based");
  assert(column != 0 && "Column should be 1-based");
  json_.emitValue(line - 1);
  json_.emitValue(column - 1);
}

void HeapSnapshot::addSample(
    std::chrono::microseconds timestamp,
    NodeID lastSeenObjectID) {
  assert(
      nextSection_ == Section::Samples && sectionOpened_ &&
      "Shouldn't be emitting samples until the sample section starts");
  assert(
      lastSeenObjectID != GCBase::IDTracker::kInvalidNode &&
      "Last seen object ID must be valid");
  json_.emitValues(
      {static_cast<uint64_t>(timestamp.count()),
       static_cast<uint64_t>(lastSeenObjectID)});
}

const char *HeapSnapshot::nodeTypeToName(NodeType type) {
  switch (type) {
#define V8_NODE_TYPE(enumerand, label) \
  case NodeType::enumerand:            \
    return #label;
#include "hermes/VM/HeapSnapshot.def"
  }
  assert(false && "Invalid NodeType");
  return "";
}

const char *HeapSnapshot::edgeTypeToName(EdgeType type) {
  switch (type) {
#define V8_EDGE_TYPE(enumerand, label) \
  case EdgeType::enumerand:            \
    return #label;
#include "hermes/VM/HeapSnapshot.def"
  }
  assert(false && "Invalid EdgeType");
  return "";
}

void HeapSnapshot::emitMeta() {
  json_.emitKey("snapshot");
  json_.openDict();

  json_.emitKey("meta");
  json_.openDict();

  json_.emitKey("node_fields");
  json_.openArray();
  json_.emitValues({
      "type",
#define V8_NODE_FIELD(label, type) #label,
#include "hermes/VM/HeapSnapshot.def"
  });
  json_.closeArray(); // node_fields

  json_.emitKey("node_types");
  json_.openArray();
  json_.openArray();
  json_.emitValues({
#define V8_NODE_TYPE(enumerand, label) label,
#include "hermes/VM/HeapSnapshot.def"
  });
  json_.closeArray();
  json_.emitValues({
#define V8_NODE_FIELD(label, type) #type,
#include "hermes/VM/HeapSnapshot.def"
  });
  json_.closeArray(); // node_types

  json_.emitKey("edge_fields");
  json_.openArray();
  json_.emitValues({
      "type",
#define V8_EDGE_FIELD(label, type) #label,
#include "hermes/VM/HeapSnapshot.def"
  });
  json_.closeArray(); // edge_fields

  json_.emitKey("edge_types");
  json_.openArray();
  json_.openArray();
  json_.emitValues({
#define V8_EDGE_TYPE(enumerand, label) label,
#include "hermes/VM/HeapSnapshot.def"
  });
  json_.closeArray();
  json_.emitValues({
#define V8_EDGE_FIELD(label, type) #type,
#include "hermes/VM/HeapSnapshot.def"
  });
  json_.closeArray(); // edge_types

  json_.emitKey("trace_function_info_fields");
  json_.openArray();
  json_.emitValues({
#define V8_TRACE_FUNCTION_INFO_FIELD(name) #name,
#include "hermes/VM/HeapSnapshot.def"
  });
  json_.closeArray(); // trace_function_info_fields

  json_.emitKey("trace_node_fields");
  json_.openArray();
  json_.emitValues({
#define V8_TRACE_NODE_FIELD(name) #name,
#include "hermes/VM/HeapSnapshot.def"
  });
  json_.closeArray(); // trace_node_fields

  json_.emitKey("sample_fields");
  json_.openArray();
  json_.emitValues({
#define V8_SAMPLE_FIELD(name) #name,
#include "hermes/VM/HeapSnapshot.def"
  });
  json_.closeArray(); // sample_fields

  json_.emitKey("location_fields");
  json_.openArray();
  json_.emitValues({
#define V8_LOCATION_FIELD(label) #label,
#include "hermes/VM/HeapSnapshot.def"
  });
  json_.closeArray(); // location_fields

  json_.closeDict(); // "meta"

  json_.emitKey("node_count");
  // This can be zero because it's only used as an optimization hint to
  // the viewer.
  json_.emitValue(0);
  json_.emitKey("edge_count");
  // This can be zero because it's only used as an optimization hint to
  // the viewer.
  json_.emitValue(0);
  json_.emitKey("trace_function_count");
  json_.emitValue(countFunctionTraceInfos());
  json_.closeDict(); // "snapshot"
}

size_t HeapSnapshot::countFunctionTraceInfos() {
  if (!stackTracesTree_) {
    return 0;
  }

  size_t count = 0;
  llvh::DenseSet<
      StackTracesTreeNode::SourceLoc,
      StackTracesTreeNode::SourceLocMapInfo>
      sourceLocSet;
  llvh::SmallVector<StackTracesTreeNode *, 128> nodeStack;
  nodeStack.push_back(stackTracesTree_->getRootNode());
  while (!nodeStack.empty()) {
    auto curNode = nodeStack.pop_back_val();
    auto funcHashToFuncIdxMapEntry = sourceLocSet.find(curNode->sourceLoc);
    if (funcHashToFuncIdxMapEntry == sourceLocSet.end()) {
      count++;
      sourceLocSet.insert(curNode->sourceLoc);
    }
    for (auto child : curNode->getChildren()) {
      nodeStack.push_back(child);
    }
  }
  return count;
}

void HeapSnapshot::emitAllocationTraceInfo() {
  if (!stackTracesTree_) {
    return;
  }

  llvh::DenseMap<
      StackTracesTreeNode::SourceLoc,
      size_t,
      StackTracesTreeNode::SourceLocMapInfo>
      sourceLocToFuncIdxMap;
  size_t nextFunctionIdx = 0;

  std::stack<
      StackTracesTreeNode *,
      llvh::SmallVector<StackTracesTreeNode *, 128>>
      nodeStack;

  beginSection(Section::TraceFunctionInfos);
  nodeStack.push(stackTracesTree_->getRootNode());
  while (!nodeStack.empty()) {
    auto curNode = nodeStack.top();
    nodeStack.pop();
    auto entry = sourceLocToFuncIdxMap.find(curNode->sourceLoc);
    if (entry == sourceLocToFuncIdxMap.end()) {
      const auto functionIdx = nextFunctionIdx++;
      sourceLocToFuncIdxMap.try_emplace(curNode->sourceLoc, functionIdx);
      // function_id needs to match the zero-based index of this function in the
      // list.
      json_.emitValue(functionIdx); // "function_id"
      json_.emitValue(curNode->name); // "name"
      json_.emitValue(curNode->sourceLoc.scriptName); // "script_name"
      json_.emitValue(curNode->sourceLoc.scriptID); // "script_id"
      // These should be emitted as 1-based, not 0-based like locations.
      json_.emitValue(curNode->sourceLoc.lineNo); // "line"
      json_.emitValue(curNode->sourceLoc.columnNo); // "column"
    }
    for (auto child : curNode->getChildren()) {
      nodeStack.push(child);
    }
  }
  endSection(Section::TraceFunctionInfos);

  beginSection(Section::TraceTree);
  nodeStack.push(stackTracesTree_->getRootNode());
  while (!nodeStack.empty()) {
    auto curNode = nodeStack.top();
    nodeStack.pop();
    if (curNode == nullptr) {
      json_.closeArray();
      continue;
    }
    json_.emitValue(curNode->id);
    auto sourceLocIdxIt = sourceLocToFuncIdxMap.find(curNode->sourceLoc);
    assert(
        sourceLocIdxIt != sourceLocToFuncIdxMap.end() &&
        "Could not find trace function info ID for sourceLoc");
    // This index must correspond to the "function_id" emitted in the
    // "trace_function_infos" section.
    json_.emitValue(sourceLocIdxIt->second); // "function_info_index"
    json_.emitValue(traceNodeStats_[curNode->id].count); // "count"
    json_.emitValue(traceNodeStats_[curNode->id].size); // "size"
    json_.openArray();
    nodeStack.push(nullptr);
    for (auto child : curNode->getChildren()) {
      nodeStack.push(child);
    }
  }
  endSection(Section::TraceTree);
}

void HeapSnapshot::emitStrings() {
  beginSection(Section::Strings);

  for (const auto &str : *stringTable_) {
    json_.emitValue(str);
  }

  endSection(Section::Strings);
}

ChromeSamplingMemoryProfile::ChromeSamplingMemoryProfile(JSONEmitter &json)
    : json_(json) {
  json_.openDict();
}

ChromeSamplingMemoryProfile::~ChromeSamplingMemoryProfile() {
  json_.closeDict();
}

void ChromeSamplingMemoryProfile::emitTree(
    StackTracesTree *stackTracesTree,
    const llvh::DenseMap<StackTracesTreeNode *, llvh::DenseMap<size_t, size_t>>
        &sizesToCounts) {
  json_.emitKey("head");
  emitNode(
      stackTracesTree->getRootNode(),
      *stackTracesTree->getStringTable(),
      sizesToCounts);
}

void ChromeSamplingMemoryProfile::emitNode(
    StackTracesTreeNode *node,
    StringSetVector &strings,
    const llvh::DenseMap<StackTracesTreeNode *, llvh::DenseMap<size_t, size_t>>
        &sizesToCounts) {
  json_.openDict();
  json_.emitKey("callFrame");
  json_.openDict();
  json_.emitKeyValue("functionName", strings[node->name]);
  json_.emitKeyValue("scriptId", std::to_string(node->sourceLoc.scriptID));
  json_.emitKeyValue("url", strings[node->sourceLoc.scriptName]);
  // For the sampling memory profiler, lines should be 0-based. The source
  // location is 1-based, so subtract 1 here.
  json_.emitKeyValue("lineNumber", node->sourceLoc.lineNo - 1);
  json_.emitKeyValue("columnNumber", node->sourceLoc.columnNo - 1);
  json_.closeDict();

  size_t selfSize = 0;
  for (const auto &sizeAndCount : sizesToCounts.lookup(node)) {
    // Size is the key, count is the value.
    selfSize += sizeAndCount.first * sizeAndCount.second;
  }
  json_.emitKeyValue("selfSize", selfSize);
  json_.emitKeyValue("id", node->id);

  json_.emitKey("children");
  json_.openArray();
  for (StackTracesTreeNode *child : node->getChildren()) {
    emitNode(child, strings, sizesToCounts);
  }
  json_.closeArray();
  json_.closeDict();
}

void ChromeSamplingMemoryProfile::beginSamples() {
  json_.emitKey("samples");
  json_.openArray();
}

void ChromeSamplingMemoryProfile::emitSample(
    size_t size,
    StackTracesTreeNode *node,
    uint64_t id) {
  json_.openDict();
  json_.emitKeyValue("size", size);
  json_.emitKeyValue("nodeId", node->id);
  json_.emitKeyValue("ordinal", id);
  json_.closeDict();
}

void ChromeSamplingMemoryProfile::endSamples() {
  json_.closeArray();
}

#endif

std::string converter(const char *name) {
  return std::string(name);
}
std::string converter(unsigned index) {
  return std::to_string(index);
}
std::string converter(int index) {
  return std::to_string(index);
}
std::string converter(const StringPrimitive *str) {
  llvh::SmallVector<char16_t, 16> buf;
  str->appendUTF16String(buf);
  std::string out;
  convertUTF16ToUTF8WithReplacements(out, UTF16Ref(buf));
  return out;
}
std::string converter(const UTF16Ref ref) {
  std::string out;
  convertUTF16ToUTF8WithReplacements(out, ref);
  return out;
}

} // namespace vm
} // namespace hermes
