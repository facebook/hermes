/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/HeapSnapshot.h"

#include "hermes/Support/Conversions.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/Support/UTF8.h"
#include "hermes/VM/StringPrimitive.h"

#include <iomanip>
#include <sstream>

namespace hermes {
namespace vm {

namespace {
void pretty(llvm::raw_ostream &os, bool compact, const char *msg) {
  if (!compact) {
    os << msg;
  }
}
} // namespace

/// @name FacebookHeapSnapshot
/// @{
FacebookHeapSnapshot::FacebookHeapSnapshot(
    llvm::raw_ostream &os,
    bool compact,
    gcheapsize_t totalHeapSize)
    : os_(os), compact_(compact) {
  os_ << "{";
  pretty(os_, compact_, "\n\t");
  os_ << "\"version\":";
  pretty(os_, compact_, " ");
  os_ << version << ",";
  pretty(os_, compact_, "\n\t");
  os_ << "\"totalHeapSize\":";
  pretty(os_, compact_, " ");
  os_ << totalHeapSize << ",";
  pretty(os_, compact_, "\n\t");
}

FacebookHeapSnapshot::~FacebookHeapSnapshot() {
  pretty(os_, compact_, "\n");
  os_ << "}";
  pretty(os_, compact_, "\n");
}

void FacebookHeapSnapshot::beginRoots() {
  os_ << "\"roots\":";
  pretty(os_, compact_, " ");
  os_ << "[";
  firstProperty_ = true;
}

void FacebookHeapSnapshot::endRoots() {
  pretty(os_, compact_, "\n\t");
  os_ << "],";
  pretty(os_, compact_, "\n\t");
}

void FacebookHeapSnapshot::beginRefs() {
  os_ << "\"refs\":";
  pretty(os_, compact_, " ");
  os_ << "{";
  pretty(os_, compact_, "\n");
}

void FacebookHeapSnapshot::endRefs() {
  pretty(os_, compact_, "\n\t");
  os_ << "},";
  pretty(os_, compact_, "\n\t");
}

void FacebookHeapSnapshot::beginIdTable() {
  firstIdTableEntry_ = true;
  os_ << "\"idtable\":";
  pretty(os_, compact_, " ");
  os_ << "[";
}

void FacebookHeapSnapshot::addIdTableEntry(UTF16Ref entry, uint32_t id) {
  if (firstIdTableEntry_) {
    firstIdTableEntry_ = false;
  } else {
    os_ << ",";
    pretty(os_, compact_, " ");
  }
  os_ << "[";
  os_ << id;
  os_ << ",";
  pretty(os_, compact_, " ");
  os_ << "\"";
  os_ << escapeJSON(converter(entry));
  os_ << "\"]";
}

void FacebookHeapSnapshot::endIdTable() {
  os_ << "]";
}
void FacebookHeapSnapshot::addRoot(ObjectID id) {
  if (!firstProperty_) {
    os_ << ",";
  } else {
    firstProperty_ = false;
  }
  pretty(os_, compact_, "\n\t\t");
  os_ << "\"" << id << "\"";
}

void FacebookHeapSnapshot::startObject(Object &&o) {
  startObject(std::move(o), nullptr);
}

void FacebookHeapSnapshot::startObject(Object &&o, const char *value) {
  if (!firstObj_) {
    os_ << ",";
    pretty(os_, compact_, "\n");
  } else {
    firstObj_ = false;
  }
  pretty(os_, compact_, "\t\t");
  os_ << "\"" << o.id << "\":";
  pretty(os_, compact_, " ");
  os_ << "{\"size\":";
  pretty(os_, compact_, " ");
  os_ << o.size << ",";
  pretty(os_, compact_, " ");
  os_ << "\"type\":";
  pretty(os_, compact_, " ");
  os_ << "\"" << cellKindStr(o.type) << "\",";
  pretty(os_, compact_, " ");
  if (value) {
    os_ << "\"value\":";
    pretty(os_, compact_, " ");
    os_ << value << ",";
    pretty(os_, compact_, " ");
  }

  os_ << "\"props\":";
  pretty(os_, compact_, " ");
  os_ << "[";
  firstProperty_ = true;
}

void FacebookHeapSnapshot::startProperty(const char *name) {
  if (!firstProperty_) {
    os_ << ",";
    pretty(os_, compact_, " ");
  } else {
    firstProperty_ = false;
  }
  if (name) {
    os_ << "[\"" << name << "\",";
  } else {
    os_ << "[null,";
  }
  pretty(os_, compact_, " ");
}

void FacebookHeapSnapshot::addToCurrentObject(const char *name, ObjectID id) {
  startProperty(name);
  os_ << "\"" << id << "\"]";
}

void FacebookHeapSnapshot::addHermesValueToCurrentObject(
    const char *name,
    HermesValue &hv) {
  if (hv.isBool()) {
    addValueToCurrentObject(name, hv.getBool());
  } else if (hv.isNumber()) {
    addValueToCurrentObject(name, hv.getNumber());
  } else if (hv.isUndefined()) {
    addUndefinedToCurrentObject(name);
  } else if (hv.isNativeValue()) {
    addNativeValueToCurrentObject(name, hv.getNativeValue());
  } else if (hv.isEmpty()) {
    addEmptyToCurrentObject(name);
  } else if (hv.isNull()) {
    addNullToCurrentObject(name);
  }
}

void FacebookHeapSnapshot::addSymbolIdToCurrentObject(
    const char *name,
    uint32_t value) {
  startProperty(name);
  os_ << "[" << value << "]]";
}

void FacebookHeapSnapshot::addValueToCurrentObject(
    const char *name,
    double value) {
  startProperty(name);
  if (std::isfinite(value)) {
    char buff[NUMBER_TO_STRING_BUF_SIZE];
    numberToString(value, buff, sizeof(buff));
    os_ << "[" << buff << "]]";
  } else {
    // Quote non finite doubles as they aren't valid JSON
    os_ << "[\"" << value << "\"]]";
  }
}

void FacebookHeapSnapshot::addValueToCurrentObject(
    const char *name,
    bool value) {
  startProperty(name);
  os_ << "[" << value << "]]";
}

void FacebookHeapSnapshot::addNullToCurrentObject(const char *name) {
  startProperty(name);
  os_ << "[null]]";
}

void FacebookHeapSnapshot::addUndefinedToCurrentObject(const char *name) {
  startProperty(name);
  os_ << "[]]";
}

void FacebookHeapSnapshot::addNativeValueToCurrentObject(
    const char *name,
    uint64_t value) {
  startProperty(name);
  os_ << "[\"0x";
  os_.write_hex(value);
  os_ << "\"]]";
}

void FacebookHeapSnapshot::addEmptyToCurrentObject(const char *name) {
  startProperty(name);
  os_ << "[\"empty\"]]";
}

void FacebookHeapSnapshot::addInternalToCurrentObject(
    const char *name,
    uint64_t value) {
  startProperty(name);
  os_ << "[\"" << value << "\"]]";
}

void FacebookHeapSnapshot::endObject() {
  os_ << "]}";
}

/// @}

/// @name rawHeapSnapshot
/// @{

void rawHeapSnapshot(
    llvm::raw_ostream &os,
    const char *start,
    const char *end) {
  // Write the start address value first.
  os << reinterpret_cast<uintptr_t>(start);
  // Copy the raw bytes of the address range into the ostream.
  os.write(start, end - start);
}

/// @}

const uint32_t V8_SNAPSHOT_NODE_FIELD_COUNT = 6;

V8HeapSnapshot::V8HeapSnapshot(JSONEmitter &json) : json_(json) {
  json_.openDict();
  emitMeta();
}

V8HeapSnapshot::~V8HeapSnapshot() {
  assert(didWriteSection(Section::Nodes));
  assert(didWriteSection(Section::Edges));
  assert(didWriteSection(Section::TraceFunctionInfos));
  assert(didWriteSection(Section::TraceTree));
  assert(didWriteSection(Section::Samples));
  assert(didWriteSection(Section::Locations));
  assert(didWriteSection(Section::Strings));
  json_.closeDict(); // top level
}

void V8HeapSnapshot::beginNodes() {
  beginSection(Section::Nodes);
  json_.emitKey("nodes");
  json_.openArray();
}

void V8HeapSnapshot::addNode(Node &&node) {
  assert(currentSection_ == Section::Nodes);
  auto res = nodeToIndex_.try_emplace(node.id, nodeCount_++);
  assert(res.second);
  (void)res;
  json_.emitValue(nodeTypeIndex(node.type));
  json_.emitValue(node.name);
  json_.emitValue(node.id);
  json_.emitValue(node.selfSize);
  json_.emitValue(node.edgeCount);
  json_.emitValue(node.traceNodeId);
#ifndef NDEBUG
  expectedEdges_ += node.edgeCount;
#endif
}

void V8HeapSnapshot::endNodes() {
  endSection(Section::Nodes);
  json_.closeArray();
}

void V8HeapSnapshot::beginEdges() {
  assert(didWriteSection(Section::Nodes));
  beginSection(Section::Edges);
  json_.emitKey("edges");
  json_.openArray();
}

void V8HeapSnapshot::addEdge(Edge &&edge) {
  assert(edgeCount_ < expectedEdges_ && "Added more edges than were expected");
#ifndef NDEBUG
  edgeCount_++;
#endif
  assert(currentSection_ == Section::Edges);
  json_.emitValue(edgeTypeIndex(edge.type));
  switch (edge.pointerKind) {
    case Edge::PKNamed:
      json_.emitValue(edge.name());
      break;
    case Edge::PKUnnamed:
      json_.emitValue(edge.index());
      break;
  }
  auto nodeIt = nodeToIndex_.find(edge.toNode);
  assert(nodeIt != nodeToIndex_.end());
  // Point to the beginning of the target node in the `nodes` flat array.
  json_.emitValue(nodeIt->second * V8_SNAPSHOT_NODE_FIELD_COUNT);
}

void V8HeapSnapshot::endEdges() {
  endSection(Section::Edges);
  json_.closeArray();
  assert(
      edgeCount_ == expectedEdges_ && "Fewer edges added than were expected");
}

void V8HeapSnapshot::emitMeta() {
  json_.emitKey("snapshot");
  json_.openDict();

  json_.emitKey("meta");
  json_.openDict();

  json_.emitKey("node_fields");
  json_.openArray();
  // NOTE: Keep this in sync with V8_SNAPSHOT_NODE_FIELD_COUNT.
  json_.emitValue("type");
  json_.emitValue("name");
  json_.emitValue("id");
  json_.emitValue("self_size");
  json_.emitValue("edge_count");
  json_.emitValue("trace_node_id");
  json_.closeArray(); // node_fields

  json_.emitKey("node_types");
  json_.openArray();
  json_.openArray();
  for (int objType = 0; objType < static_cast<int>(Node::Type::NumTypes);
       objType++) {
    json_.emitValue(Node::nodeTypeStr(static_cast<Node::Type>(objType)));
  }
  json_.closeArray();
  json_.emitValues({"string", "number", "number", "number", "number"});
  json_.closeArray(); // node_types

  json_.emitKey("edge_fields");
  json_.openArray();
  json_.emitValues({"type", "name_or_index", "to_node"});
  json_.closeArray(); // edge_fields

  json_.emitKey("edge_types");
  json_.openArray();
  json_.openArray();
  // NOTE: Keep this in sync with the members of Edge::Type.
  json_.emitValues({"context",
                    "element",
                    "property",
                    "internal",
                    "hidden",
                    "shortcut",
                    "weak"});
  json_.closeArray();
  json_.emitValues({"string_or_number", "node"});
  json_.closeArray(); // edge_types

  json_.emitKey("trace_function_info_fields");
  json_.openArray();
  // TODO: Possibly populate this if Chrome complains
  json_.closeArray(); // trace_function_info_fields

  json_.emitKey("trace_node_fields");
  json_.openArray();
  // TODO: Possibly populate this if Chrome complains
  json_.closeArray(); // trace_node_fields

  json_.emitKey("sample_fields");
  json_.openArray();
  // TODO: Possibly populate this if Chrome complains
  json_.closeArray(); // sample_fields

  json_.emitKey("location_fields");
  json_.openArray();
  // TODO: Possibly populate this if Chrome complains
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
  json_.emitValue(0);
  json_.closeDict(); // "snapshot"
}

void V8HeapSnapshot::beginTraceFunctionInfos() {
  assert(didWriteSection(Section::Edges));
  beginSection(Section::TraceFunctionInfos);
  json_.emitKey("trace_function_infos");
  json_.openArray();
}

void V8HeapSnapshot::endTraceFunctionInfos() {
  endSection(Section::TraceFunctionInfos);
  json_.closeArray();
}

void V8HeapSnapshot::beginTraceTree() {
  assert(didWriteSection(Section::TraceFunctionInfos));
  beginSection(Section::TraceTree);
  json_.emitKey("trace_tree");
  json_.openArray();
}

void V8HeapSnapshot::endTraceTree() {
  endSection(Section::TraceTree);
  json_.closeArray();
}

void V8HeapSnapshot::beginSamples() {
  assert(didWriteSection(Section::TraceTree));
  beginSection(Section::Samples);
  json_.emitKey("samples");
  json_.openArray();
}

void V8HeapSnapshot::endSamples() {
  endSection(Section::Samples);
  json_.closeArray();
}

void V8HeapSnapshot::beginLocations() {
  assert(didWriteSection(Section::Samples));
  beginSection(Section::Locations);
  json_.emitKey("locations");
  json_.openArray();
}

void V8HeapSnapshot::endLocations() {
  endSection(Section::Locations);
  json_.closeArray();
}

void V8HeapSnapshot::beginStrings() {
  assert(didWriteSection(Section::Locations));
  beginSection(Section::Strings);
  json_.emitKey("strings");
  json_.openArray();
}

void V8HeapSnapshot::addString(llvm::StringRef str) {
  assert(currentSection_ == Section::Strings);
  json_.emitValue(str);
}

void V8HeapSnapshot::endStrings() {
  endSection(Section::Strings);
  json_.closeArray();
}

void V8HeapSnapshot::beginSection(Section section) {
  assert(!currentSection_.hasValue());
  assert(!sectionsWritten_[sectionIndex(section)]);
  currentSection_ = section;
}

void V8HeapSnapshot::endSection(Section section) {
  assert(currentSection_.hasValue() && currentSection_.getValue() == section);
  currentSection_ = llvm::None;
  sectionsWritten_.set(sectionIndex(section));
}

bool V8HeapSnapshot::didWriteSection(Section section) const {
  return sectionsWritten_[sectionIndex(section)];
}

size_t V8HeapSnapshot::sectionIndex(Section section) {
  return static_cast<std::underlying_type<Section>::type>(section);
}

uint32_t V8HeapSnapshot::nodeTypeIndex(V8HeapSnapshot::Node::Type type) {
  return static_cast<uint32_t>(type);
}

uint32_t V8HeapSnapshot::edgeTypeIndex(V8HeapSnapshot::Edge::Type type) {
  return static_cast<uint32_t>(type);
}

const char *V8HeapSnapshot::Node::nodeTypeStr(Node::Type type) {
  static const char *strs[] = {
      "hidden",
      "array",
      "string",
      "object",
      "code",
      "closure",
      "regexp",
      "number",
      "native",
      "synthetic",
      "concatenated string",
      "sliced string",
      "symbol",
      "bigint",
  };
  return strs[static_cast<int>(type)];
}

V8HeapSnapshot::Node::Type V8HeapSnapshot::Node::cellKindToType(CellKind kind) {
  if (kindInRange(
          kind,
          CellKind::StringPrimitiveKind_first,
          CellKind::StringPrimitiveKind_last)) {
    return Type::String;
  } else if (kind == CellKind::ArrayStorageKind) {
    // The array type is meant to be used by primitive internal array
    // constructs. User-creatable arrays should be Object.
    return Type::Array;
  } else {
    return Type::Object;
  }
}

std::string escapeJSON(llvm::StringRef s) {
  std::ostringstream o;
  for (const unsigned char c : s) {
    switch (c) {
      case '"':
        o << "\\\"";
        break;
      case '\\':
        o << "\\\\";
        break;
      case '\b':
        o << "\\b";
        break;
      case '\f':
        o << "\\f";
        break;
      case '\n':
        o << "\\n";
        break;
      case '\r':
        o << "\\r";
        break;
      case '\t':
        o << "\\t";
        break;
      default:
        if (c <= '\x1f') {
          o << "\\u" << std::hex << std::setw(4) << std::setfill('0')
            << static_cast<int>(c);
        } else {
          o << c;
        }
    }
  }
  return o.str();
}

std::string converter(const char *name) {
  return std::string(name);
}
std::string converter(unsigned index) {
  return oscompat::to_string(index);
}
std::string converter(int index) {
  return oscompat::to_string(index);
}
std::string converter(const StringPrimitive *str) {
  llvm::SmallVector<char16_t, 16> buf;
  str->copyUTF16String(buf);
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
